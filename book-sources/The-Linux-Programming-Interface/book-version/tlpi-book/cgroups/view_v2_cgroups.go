/* view_v2_cgroups.go

   Copyright (C) Michael Kerrisk, 2018

   Licensed under GNU General Public License version 3 or later

   Display one or more subtrees in the cgroups v2 hierarchy.  The following
   info is displayed for each cgroup: the cgroup type, the controllers enabled
   in the cgroup, and the process and thread members of the cgroup.
*/

package main

import (
	"bufio"
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"syscall"
	"unsafe"
)

// Info from command-line options

type CmdLineOptions struct {
	useColor  bool // Use color in the output
	showPids  bool // Show member PIDs for each cgroup
	showTids  bool // Show member TIDs for each cgroup
	showOwner bool // Show cgroup ownership
}

var opts CmdLineOptions

// 'rootSlashCnt' is the number of slashes in the pathname of the cgroup that
// is the root of the subtree that is currently being displayed.  This is used
// for calculating the indent for displaying the descendant cgroups under this
// root.

var rootSlashCnt int

// Some terminal color sequences for coloring the output.

const ESC = ""
const RED = ESC + "[31m"
const YELLOW = ESC + "[93m"
const BOLD = ESC + "[1m"
const LIGHT_BLUE = ESC + "[38;5;51m"
const GREEN = ESC + "[92m"
const BLUE = ESC + "[34m"
const MAGENTA = ESC + "[35m"
const LIGHT_PURPLE = ESC + "[38;5;93m"
const NORMAL = ESC + "(B" + ESC + "[m"
const GRAY = ESC + "[38;5;240m"
const BRIGHT_YELLOW = ESC + "[93m"
const BLINK = ESC + "[5m"
const REVERSE = ESC + "[7m"
const UNDERLINE = ESC + "[4m"

// A map defining the color used to display the different cgroup types.

var cgroupColor = map[string]string{
	"root":            "",
	"domain":          "",
	"domain threaded": UNDERLINE + BOLD + GREEN,
	"threaded":        GREEN,
	"domain invalid":  REVERSE + LIGHT_PURPLE,
}

// A map defining the string used to display each cgroup type.

var cgroupAbbrev = map[string]string{
	"root":            "[/]",
	"domain":          "[d]",
	"domain threaded": "[dt]",
	"threaded":        "[t]",
	"domain invalid":  "[inv]",
}

func main() {
	opts = parseCmdLineOptions()

	if len(flag.Args()) == 0 {
		showUsageAndExit(1)
	}

	// Walk the directory trees specified in the command-line arguments.

	for _, f := range flag.Args() {
		f = filepath.Clean(f) // Remove consecutive + trailing slashes
		rootSlashCnt = len(strings.Split(f, "/"))

		err := filepath.Walk(f, walkFn)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
	}
}

// Callback function used by filepath.Walk() to visit each file
// in a subtree.

func walkFn(path string, fi os.FileInfo, e error) error {

	if e != nil {
		return e
	}

	if fi.IsDir() { // We're only interested in the cgroup directories
		err := displayCgroup(path)
		if err != nil {
			return err
		}
	}

	return nil
}

// displayCgroup() displays all of the info about the cgroup specified
// by 'path'.

func displayCgroup(path string) (err error) {

	var cgroupType string

	// Get the cgroup type. If this fails, the most likely reason is that
	// the 'cgroup.type' file does not exist because this is the root
	// cgroup.

	ct, err := ioutil.ReadFile(path + "/" + "cgroup.type")
	if err != nil {
		cgroupType = "root"
	} else {
		cgroupType = strings.TrimSpace(string(ct))
	}

	// Calculate indent according to number of slashes in pathname
	// (relative to the root of the currently displayed subtree).

	level := len(strings.Split(path, "/")) - rootSlashCnt
	indent := strings.Repeat(" ", 4*level)

	// At the topmost level, we display the full pathname from the
	// command line. At lower levels, we display just the basename
	// component of the pathname.

	p := path
	if level > 0 {
		p = filepath.Base(path)
	}

	// We show each cgroup type with a distinctive color/style.

	fmt.Print(indent + cgroupColor[cgroupType] + p + NORMAL + " " +
		cgroupAbbrev[cgroupType])

	// Display controllers that are enabled for this group.

	err = displayControllers(path)
	if err != nil {
		return err
	}

	fmt.Println()

	// Display cgroup ownership

	if opts.showOwner {
		fmt.Print(indent + "    ")
		err = displayCgroupOwnership(path)
		if err != nil {
			return err
		}
		fmt.Println()
	}

	// Display member processes and threads

	err = displayMembers(path, cgroupType, indent+"    ")
	if err != nil {
		return err
	}

	return nil
}

// parseCmdLineOptions() parses command-line options and returns them
// conveniently packaged in a structure.

func parseCmdLineOptions() CmdLineOptions {

	var opts CmdLineOptions

	// Parse command-line options.

	helpPtr := flag.Bool("help", false, "Show detailed usage message")
	noColorPtr := flag.Bool("no-color", false,
		"Don't use color in output display")
	noPidsPtr := flag.Bool("no-pids", false,
		"Don't show PIDs that are members of each cgroup")
	noTidsPtr := flag.Bool("no-tids", false,
		"Don't show TIDs that are members of each cgroup")
	showOwnerPtr := flag.Bool("show-owner", false,
		"Show owner UID for cgroup")

	flag.Parse()

	if *helpPtr {
		showUsageAndExit(0)
	}

	opts.useColor = !*noColorPtr
	opts.showPids = !*noPidsPtr
	opts.showTids = !*noTidsPtr
	opts.showOwner = *showOwnerPtr

	return opts
}

// showUsageAndExit() prints a command-line usage message for this program and
// terminates the program with the specified 'status' value.

func showUsageAndExit(status int) {
	fmt.Println(
		`Usage: view_v2_cgroups [options] <cgroup-dir-path>...

Show the state (cgroup type, enabled controllers, member processes, member
TIDs,and, optionally, owning UID) of the cgroups in the cgroup v2
subhierarchies whose pathnames are supplied as the command line arguments.

Options:
--no-color      Don't use color in the displayed output.
--no-pids       Don't show the member PIDs in each cgroup.
--no-tids       Don't show the member TIDs in each cgroup.
--show-owner    Show the user ID of each cgroup.
  `)

	os.Exit(status)
}

// displayCgroupOwnership() displays the ownership of a cgroup directory.

func displayCgroupOwnership(path string) error {

	fi, err := os.Stat(path)
	if err != nil {
		return err
	}

	stat, ok := fi.Sys().(*syscall.Stat_t)
	if !ok {
		return errors.New("fi.Sys() failure for " + path)
		return err
	}

	if opts.useColor {
		fmt.Print(MAGENTA)
	}

	fmt.Print("<UID: " + strconv.Itoa(int(stat.Uid)))
	//fmt.Print("; GID: " + strconv.Itoa(int(stat.Gid)))
	//fmt.Print("; " + fmt.Sprint(fi.Mode())[1:])
	fmt.Print(">")

	if opts.useColor {
		fmt.Print(NORMAL)
	}

	return nil
}

// displayControllers() displays the controllers that are enabled
// for the cgroup specified by 'path'.

func displayControllers(path string) error {

	scPath := path + "/" + "cgroup.subtree_control"
	sc, err := ioutil.ReadFile(scPath)
	if err != nil {
		return err
	}

	controllers := strings.TrimSpace(string(sc)) // Trim trailing newline
	if controllers != "" {
		controllers = "(" + controllers + ")"
		if opts.useColor {
			controllers = BRIGHT_YELLOW + controllers + NORMAL
		}
		fmt.Print("    " + controllers)
	}

	return nil
}

// displayMembers() displays the member processes and member threads of the
// cgroup specified by 'path'.

func displayMembers(path string, cgroupType string, indent string) error {

	// Calculate display width of PID and TID lists.

	const minDisplayWidth = 32
	width := getTerminalWidth() - len(indent)
	if width < minDisplayWidth {
		width = minDisplayWidth
	}

	// If this cgroup has member processes, display them. The
	// 'cgroup.procs' file is not readable in "threaded" cgroups.

	if cgroupType != "threaded" && opts.showPids {
		err := displayProcesses(path, width, indent)
		if err != nil {
			return err
		}
	}

	// Display member threads.

	if opts.showTids {
		err := displayThreads(path, width, indent)
		if err != nil {
			return err
		}
	}

	return nil
}

// Discover width of terminal, so that we can format output suitably.

func getTerminalWidth() int {
	type winsize struct {
		row    uint16
		col    uint16
		xpixel uint16
		ypixel uint16
	}
	var ws winsize

	ret, _, _ := syscall.Syscall(syscall.SYS_IOCTL,
		uintptr(syscall.Stdout), uintptr(syscall.TIOCGWINSZ),
		uintptr(unsafe.Pointer(&ws)))

	if int(ret) == -1 { // Call failed (perhaps stdout is not a terminal)
		return 80
	}

	return int(ws.col)
}

// displayProcesses() displays the set of processes that are members of the
// cgroup 'path'.

func displayProcesses(path string, width int, indent string) error {
	pids, err := getSortedIntsFrom(path + "/" + "cgroup.procs")
	if err != nil {
		return err
	}

	if len(pids) > 0 {
		buf := strconv.Itoa(pids[0])
		for _, p := range pids[1:] {
			buf += " " + strconv.Itoa(p)
		}

		buf = wrapText(buf+"}", "PIDs: {", width, indent)

		if opts.useColor {
			buf = colorEachLine(buf, LIGHT_BLUE)
		}

		fmt.Println(buf)
	}

	return nil
}

// displayThreads() displays the set of threads that are members of the
// cgroup 'path'.

func displayThreads(path string, width int, indent string) error {

	tids, err := getSortedIntsFrom(path + "/" + "cgroup.threads")
	if err != nil {
		return err
	}

	if len(tids) == 0 {
		return nil
	}

	buf := ""
	for i, t := range tids {
		if i > 0 {
			buf += " "
		}

		// Discover the thread group ID (PID) of this thread.

		tgid, err := getTgid(t)
		if err != nil {
			return err
		}

		// Determine whether this thread is scheduled under a realtime
		// scheduling policy. We do this because the cgroups v2 'cpu'
		// controller doesn't yet understand realtime threads.
		// Consequently, all such threads must be placed in the root
		// cgroup before the 'cpu' controller can be enabled. In order
		// to highlight the presence of realtime threads in nonroot
		// cgroups, we display these threads with a distinctive marker.

		isRealtime, err := getPolicy(t)
		if err != nil {
			return err
		}

		buf += fmt.Sprint(t)
		if isRealtime {
			buf += "*"
		}
		if tgid != t {
			buf += "-[" + fmt.Sprint(tgid) + "]"
		}
	}

	buf = wrapText(buf+"}", "TIDs: {", width, indent)

	if opts.useColor {

		// Highlight the marker character used for realtime threads.

		replacer := strings.NewReplacer("*", RED+REVERSE+"*"+
			NORMAL+LIGHT_BLUE)
		buf = replacer.Replace(buf)

		buf = colorEachLine(buf, LIGHT_BLUE)
	}

	fmt.Println(buf)

	return nil
}

// getPolicy() returns a flag indicating whether the thread with the specified
// TID is scheduled under a realtime policy.

func getPolicy(tid int) (bool, error) {

	const SCHED_FIFO = 1
	const SCHED_RR = 2
	const SCHED_DEADLINE = 6

	type sched_param struct {
		sched_priority uint32
	}

	var sp sched_param
	var policy int

	ret, _, e := syscall.Syscall6(syscall.SYS_SCHED_GETSCHEDULER,
		uintptr(tid), uintptr(unsafe.Pointer(&sp)),
		uintptr(0), uintptr(0), uintptr(0), uintptr(0))

	policy = int(ret)

	if policy == -1 {
		return false, e
	}

	isRealtime := policy == SCHED_DEADLINE || policy == SCHED_FIFO ||
		policy == SCHED_RR

	return isRealtime, nil
}

// getTgid() obtains the thread group ID (PID) of the thread 'tid'
// by looking up the appropriate field in the /proc/TID/status file.

func getTgid(tid int) (int, error) {
	sfile := "/proc/" + strconv.Itoa(tid) + "/status"

	file, err := os.Open(sfile)
	if err != nil {

		// Probably, the thread terminated between the time we
		// accessed the namespace files and the time we tried to open
		// /proc/TID/status.

		return 0, err
	}

	defer file.Close() // Close file on return from this function.

	// Scan file line by line, looking for 'Tgid:' entry.

	re := regexp.MustCompile(":[ \t]*")

	s := bufio.NewScanner(file)
	for s.Scan() {
		match, _ := regexp.MatchString("^Tgid:", s.Text())
		if match {
			tokens := re.Split(s.Text(), -1)
			tgid, _ := strconv.Atoi(tokens[1])
			return tgid, nil
		}
	}

	// There should always be a 'Tgid:' entry, but just in case there
	// is not...

	e := errors.New("Error scanning )" + sfile +
		": could not find 'Tgid' field")
	return 0, e
}

// getSortedIntsFrom() reads the contents of 'path', which should be a file
// containing white-space delimited integers, and returns those integers as
// a sorted slice.

func getSortedIntsFrom(path string) ([]int, error) {
	buf, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}

	if len(buf) == 0 {
		return nil, nil
	}

	slist := strings.Split(strings.TrimSpace(string(buf)), "\n")

	var list []int
	for _, s := range slist {
		i, _ := strconv.Atoi(s)
		list = append(list, i)
	}

	sort.Ints(list)

	return list, nil
}

// colorEachLine() puts a terminal color sequence just before the first
// non-white-space character in each line of 'buf', and places the terminal
// sequence to return the terminal color to white at the end of each line.

func colorEachLine(buf string, color string) string {
	re := regexp.MustCompile(`( *)(.*)`)
	return re.ReplaceAllString(buf, "$1"+color+"$2"+NORMAL)
}

// Return wrapped version of text in 'text' by adding newline characters
// on white space boundaries at most 'width' characters apart. Each
// wrapped line is prefixed by the specified 'indent' (whose size is *not*
// included as part of 'width' for the purpose of the wrapping algorithm).
// The first line of output is additionally prefixed by the string in 'prefix',
// and subsequent lines are also additionally prefixed by an equal amount of
// white space.

func wrapText(text string, prefix string, width int, indent string) string {

	// Break up text on white space to produce a slice of words.

	words := strings.Fields(text)

	// If there were no words, return an empty string.

	if len(words) == 0 {
		return ""
	}

	result := indent + prefix + words[0]
	col := len(prefix) + len(words[0])
	width -= len(prefix)
	indent += strings.Repeat(" ", len(prefix))

	for _, word := range words[1:] {
		if col+len(word)+1 > width { // Overflow ==> start on new line
			result += "\n" + indent + word
			col = len(word)
		} else {
			result += " " + word
			col += 1 + len(word)
		}
	}

	return result
}
