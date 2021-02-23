/* namespaces_of.go

   Copyright (C) Michael Kerrisk, 2018

   Licensed under GNU General Public License version 3 or later

   Show the namespace memberships of one or more processes in the context
   of the user or PID namespace hierarchy.

   This program does one of the following:
   * If provided with a list of PIDs, this program shows the namespace
     memberships of those processes.
   * If no PIDs are provided, the program shows the namespace memberships
     of all processes on the system (which it discovers by parsing the
     /proc/PID/ns directories).
   * If no PIDs are provided, and the "--subtree=<pid>" option is specified,
     then the program shows the subtree of the PID or user namespace hierarchy
     that is rooted at the namespace of the specified PID.

   By default, the program shows namespace memberships in the context of the
   user namespace hierarchy, showing also the nonuser namespaces owned by
   each user namespace. If the "--pidns" option is specified, the program
   instead shows just the PID namespace hierarchy.

   The "--no-pids" option suppresses the display of the processes that
   are members of each namespace.

   The "--show-comm" option displays the command being run by each process.

   The "--all-pids" option can be used in conjunction with "--pidns",
   so that for each process that is displayed, its PIDs in all of the PID
   namespaces of which it is a member are shown.

   The "--no-color" option can be used to suppress the use of color
   in the displayed output.

   When displaying the user namespace hierarchy, the "--namespaces=<list>"
   option can be used to specify a list of the nonuser namespace types to
   include in the displayed output; the default is to include all nonuser
   namespace types.

   This program discovers the namespaces on the system, and their
   relationships, by scanning /proc/PID/ns/* symlink files and matching the
   device IDs and inode numbers of those files using the operations described
   in ioctl_ns(2).  In cases where the program must inspect symlink files of
   processes that are owned by other users, the program must be run as
   superuser.

   As described in clone(2), the CLONE_THREAD flag can't be specified in
   conjunction with either CLONE_NEWUSER or CLONE_NEWPID. This means that
   all of the threads in a multithreaded process must be in the same user
   and PID namespaces. Therefore, it is not necessary to scan the
   /proc/PID/task/TID/ns directories to discover any further information
   about the shape of the user or PID namespace hierarchy.
*/

package main

import (
	"bufio"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"syscall"
	"unsafe"
)

// The following structure stores info from command-line options.

type CmdLineOptions struct {
	useColor           bool   // Use color in the output
	showCommand        bool   // Show the command being run by each process
	showPids           bool   // Show member PIDs for each namespace
	showAllPids        bool   // Show all of a process's PIDs (PID NS only)
	showPidnsHierarchy bool   // Display the PID namespace hierarchy
	subtreePID         string // Display hierarchy rooted at specific PID
	namespaces         int    // Bit mask of CLONE_NEW* values
}

// A namespace is uniquely identified by the combination of a device ID
// and an inode number.

type NamespaceID struct {
	device uint64 // dev_t
	inode  uint64 // ino_t
}

// For each namespace, we record a number of attributes, beginning with the
// namespace type and the PIDs of the processes that are members of the
// namespace. In the case of user namespaces, we also record (a) the nonuser
// namespaces that the namespace owns and (b) the user namespaces that are
// parented by this namespace; alternatively, if the "--pidns" option was
// specified, we record just the namespaces that are parented by this
// namespace. Finally, for user namespaces, we record the UID of the namespace
// creator.

type NamespaceAttribs struct {
	nsType     int           // CLONE_NEW*
	pids       []int         // Member processes
	children   []NamespaceID // Child+owned namespaces (user/PID NSs only)
	creatorUID int           // UID of creator (user NSs only)
	uidMap     string        // UID map (user NSs only)
	gidMap     string        // UID map (user NSs only)
}

type NamespaceList map[NamespaceID]*NamespaceAttribs

// The 'NamespaceInfo' structure records information about the namespaces
// that have been discovered:
// * The 'nsList' map records all of the namespaces that we visit.
// * While adding the first namespace to 'nsList', we'll discover the ancestor
//   of all namespaces (the root of the user or PID namespace hierarchy).
//   We record the ID of that namespace in 'rootNS'.
// * We may encounter nonuser namespaces whose user namespace owners are not
//   visible because they are ancestors of the user's user namespace (i.e.,
//   this program is being run from a noninitial user namespace, in a shell
//   started by a command such as "unshare -Uripf --mount-proc"). We record
//   these namespaces as being children of a special entry in the 'nsList' map,
//   with the key 'invisUserNS'. (The implementation of this special entry
//   presumes that there is no namespace file that has device ID 0 and inode
//   number 0.)

type NamespaceInfo struct {
	nsList NamespaceList
	rootNS NamespaceID
}

var invisUserNS = NamespaceID{0, 0} // Const value

// Namespace ioctl() operations (see ioctl_ns(2)).

const NS_GET_USERNS = 0xb701    // Get owning user NS (or parent of user NS)
const NS_GET_PARENT = 0xb702    // Get parent NS (for user or PID NS)
const NS_GET_NSTYPE = 0xb703    // Return namespace type (see below)
const NS_GET_OWNER_UID = 0xb704 // Return creator UID for user NS

// Namespace types returned by NS_GET_NSTYPE.

const CLONE_NEWNS = 0x00020000
const CLONE_NEWCGROUP = 0x02000000
const CLONE_NEWUTS = 0x04000000
const CLONE_NEWIPC = 0x08000000
const CLONE_NEWUSER = 0x10000000
const CLONE_NEWPID = 0x20000000
const CLONE_NEWNET = 0x40000000
const CLONE_NEWTIME = 0x00000080

// The following map lists all known namespaces, mapping the CLONE_* constant
// name to the corresponding symlink filename in /proc/PID/ns.
//
// In order to support a a new namespace type when it is added to the kernel,
// it should suffice to just add an entry in this map.

var namespaceMap = map[int]string{
	CLONE_NEWCGROUP: "cgroup",
	CLONE_NEWIPC:    "ipc",
	CLONE_NEWNS:     "mnt",
	CLONE_NEWNET:    "net",
	CLONE_NEWPID:    "pid",
	CLONE_NEWTIME:   "time",
	CLONE_NEWUSER:   "user",
	CLONE_NEWUTS:    "uts",
}

// 'namespaceMap' was initialized with a list of all of the currently known
// namespace symlink names. However, we might be running on an older kernel
// that does not know about some newer namespaces (e.g., time namespaces). So,
// we build a list of the namespace symlinks that are actually available on
// this system by checking the names in 'namespaceMap' against what exists in
// /proc/self/ns to build the subset of namespace symlinks that actually exist
// on this system.

func getActuaNamespaceSymlinkNames() []string {
	var res = []string{}

	for _, nsFile := range namespaceMap {
		if _, err := os.Stat("/proc/self/ns/" + nsFile); err == nil {
			res = append(res, nsFile)
		}
	}

	return res
}

// The set of the namespace symlink files in the /proc/PID/ns
// directory that actually exist on this running kernel.

var namespaceSymlinkNames []string = getActuaNamespaceSymlinkNames()

// Define some terminal escape sequences for displaying color output.

const ESC = ""
const RED = ESC + "[31m"
const YELLOW = ESC + "[93m"
const BOLD = ESC + "[1m"
const LIGHT_BLUE = ESC + "[38;5;51m"
const NORMAL = ESC + "(B" + ESC + "[m"
const PID_COLOR = LIGHT_BLUE
const USERNS_COLOR = YELLOW + BOLD

// Create and return a new namespace ID using the device ID and inode
// number of the namespace referred to by 'namespaceFD'.

func newNamespaceID(namespaceFD int) NamespaceID {
	var sb syscall.Stat_t
	var err error

	// Obtain the device ID and inode number of the namespace file.
	// These values together form the key for the 'nsList' map entry.

	err = syscall.Fstat(namespaceFD, &sb)
	if err != nil {
		fmt.Println("syscall.Fstat():", err)
		os.Exit(1)
	}

	return NamespaceID{sb.Dev, sb.Ino}
}

// addNamespace() adds the namespace referred to by the file descriptor
// 'namespaceFD' to the 'nsi.nsList' map (creating an entry in the map if one
// does not already exist) and, if 'pid' is greater than zero, adds the PID it
// contains to the list of PIDs that are resident in the namespace.
//
// This function is recursive (via the helper function addNamespaceToList()),
// calling itself to ensure that an entry is also created for the parent or
// owning namespace of the namespace referred to by 'namespaceFD'. Once that
// has been done, the namespace referred to by 'namespaceFD' is made a child
// of the parent/owning namespace. Note that, except in the case of the
// initial namespace, a parent/owning namespace must exist, since it is pinned
// into existence by the existence of the child/owned namespace (and that
// namespace is in turn pinned into existence by the open file descriptor
// 'namespaceFD').
//
// 'pid' is a PID to be added to the list of PIDs resident in this namespace.
// When called recursively to create the ancestor namespace entries, this
// function is called with 'pid' as -1, meaning that no PID needs to be added
// for this namespace entry.
//
// The return value of the function is the ID of the namespace entry
// (i.e., the device ID and inode number corresponding to the namespace
// file referred to by 'namespaceFD').

func (nsi *NamespaceInfo) addNamespace(namespaceFD int, pid int,
	opts CmdLineOptions) NamespaceID {

	ns := newNamespaceID(namespaceFD)

	// If this namespace is not already in the namespaces list of 'nsi',
	// add it to the list.

	if _, fnd := nsi.nsList[ns]; !fnd {
		nsi.addNamespaceToList(ns, namespaceFD, opts)
	}

	// Add PID to PID list for this namespace entry.

	if pid > 0 {
		nsi.nsList[ns].pids = append(nsi.nsList[ns].pids, pid)
	}

	return ns
}

// addNamespaceToList() adds the namespace 'ns' to the namespaces list
// of 'nsi'. For an explanation of the remaining arguments, see the comments
// for addNamespace().

func (nsi *NamespaceInfo) addNamespaceToList(ns NamespaceID, namespaceFD int,
	opts CmdLineOptions) {

	// Namespace entry does not yet exist in 'nsList' map; create it.

	nsi.nsList[ns] = new(NamespaceAttribs)
	nsi.nsList[ns].nsType = namespaceType(namespaceFD)

	// If this is a user namespace, record the user ID of the creator of
	// the namespace.

	if nsi.nsList[ns].nsType == CLONE_NEWUSER {
		var uid int

		ret, _, err := syscall.Syscall(syscall.SYS_IOCTL,
			uintptr(namespaceFD), uintptr(NS_GET_OWNER_UID),
			uintptr(unsafe.Pointer(&uid)))

		if (int)((uintptr)(unsafe.Pointer(ret))) == -1 {
			fmt.Println("ioctl(NS_GET_OWNER_UID):", err)
			os.Exit(1)
		}

		nsi.nsList[ns].creatorUID = uid
	}

	// Get a file descriptor for the parent/owning namespace.
	// NS_GET_USERNS returns the owning user namespace when its argument
	// is a nonuser namespace, and (conveniently) returns the parent user
	// namespace when its argument is a user namespace. On the other hand,
	// if we are handling only the PID namespace hierarchy, then we must
	// use NS_GET_PARENT to get the parent PID namespace.

	ioctlOp := NS_GET_USERNS
	if opts.showPidnsHierarchy {
		ioctlOp = NS_GET_PARENT
	}

	ret, _, err := syscall.Syscall(syscall.SYS_IOCTL,
		uintptr(namespaceFD), uintptr(ioctlOp), 0)
	parentFD := (int)((uintptr)(unsafe.Pointer(ret)))

	if parentFD == -1 {

		// Any error other than EPERM is unexpected; bail.

		if err != syscall.EPERM {
			fmt.Println("ioctl():", err)
			os.Exit(1)
		}

		// We got an EPERM error...

		if nsi.nsList[ns].nsType == CLONE_NEWUSER ||
			ioctlOp == NS_GET_PARENT {

			// If the current namespace is a user namespace and
			// NS_GET_USERNS fails with EPERM, or we are processing
			// only PID namespaces and NS_GET_PARENT fails with
			// EPERM, then this is the root namespace (or, at
			// least, the topmost visible namespace); remember it.

			nsi.rootNS = ns

		} else {

			// Otherwise, we are inspecting a nonuser namespace and
			// NS_GET_USERNS failed with EPERM, meaning that the
			// user namespace that owns this nonuser namespace is
			// not visible (i.e., is an ancestor user namespace).
			// Record these namespaces as children of a special
			// entry in the 'nsList' map.

			if _, fnd := nsi.nsList[invisUserNS]; !fnd {

				// The special parent entry does not yet exist;
				// create it.

				nsi.nsList[invisUserNS] = new(NamespaceAttribs)
				nsi.nsList[invisUserNS].nsType = CLONE_NEWUSER
			}

			nsi.nsList[invisUserNS].children =
				append(nsi.nsList[invisUserNS].children, ns)
		}

	} else {

		// The ioctl() operation successfully returned a parent/owning
		// namespace; make sure that namespace has an entry in the map.
		// Specify the 'pid' argument as -1, meaning that there is no
		// PID to be recorded as being a member of the parent/owning
		// namespace.

		parent := nsi.addNamespace(parentFD, -1, opts)

		// Make the current namespace entry a child of the
		// parent/owning namespace entry.

		nsi.nsList[parent].children =
			append(nsi.nsList[parent].children, ns)

		syscall.Close(parentFD)
	}
}

// namespaceType() returns a CLONE_NEW* constant telling us what kind of
// namespace is referred to by 'namespaceFD'.

func namespaceType(namespaceFD int) int {

	ret, _, err := syscall.Syscall(syscall.SYS_IOCTL, uintptr(namespaceFD),
		uintptr(NS_GET_NSTYPE), 0)
	nsType := (int)((uintptr)(unsafe.Pointer(ret)))
	if nsType == -1 {
		fmt.Println("ioctl(NS_GET_NSTYPE)", err)
		os.Exit(1)
	}

	return nsType
}

// addProcessNamespace() processes a single /proc/PID/ns/* entry, creating a
// namespace entry for that file and, as necessary, namespace entries for all
// ancestor namespaces going back to the initial namespace. 'pid' is a
// string containing a PID; 'nsFile' is a string identifying which namespace
// symlink to open.

func (nsi *NamespaceInfo) addProcessNamespace(pid string, nsFile string,
	opts CmdLineOptions, isCmdLineArg bool) {

	// Obtain a file descriptor that refers to the namespace
	// corresponding to 'pid' and 'nsFile'.

	namespaceFD, err := syscall.Open("/proc/"+pid+"/ns/"+nsFile,
		syscall.O_RDONLY, 0)

	if namespaceFD < 0 {

		fmt.Print("Could not open " + "/proc/" + pid + "/ns/" +
			nsFile + ": ")

		if err == syscall.EACCES {

			// We didn't have permission to open /proc/PID/ns/*.

			fmt.Println(err)
			fmt.Println("Rerun this program as superuser")
			os.Exit(1)

		} else {

			// The most likely other error is ENOENT ("no such
			// file"). We differentiate two cases when dealing with
			// the error: the specified PID came from the command
			// line or it is one of a list produced by scanning
			// /proc/PID.  In the first case, we assume that the
			// user supplied an invalid PID, diagnose an error and
			// terminate. In the second case, it may be that a
			// /proc/PID entry disappeared from under our
			// feet--that is, the process terminated while we were
			// parsing /proc. If this happens, we simply print a
			// message and carry on.

			if isCmdLineArg {
				fmt.Println(err)
				os.Exit(1)
			} else {
				fmt.Println("process terminated while we " +
					"were parsing?")
				return
			}
		}
	}

	// Add entry for this namespace, and all of its ancestor namespaces.

	npid, _ := strconv.Atoi(pid)
	nsi.addNamespace(namespaceFD, npid, opts)

	syscall.Close(namespaceFD)
}

// addNamespacesForAllProcesses() scans /proc/PID directories to build
// namespace entries in 'nsi' for all processes on the system.

func (nsi *NamespaceInfo) addNamespacesForAllProcesses(namespaces []string,
	opts CmdLineOptions) {

	// Fetch a list of the filenames under /proc.

	procFiles, err := ioutil.ReadDir("/proc")
	if err != nil {
		fmt.Println("ioutil.Readdir():", err)
		os.Exit(1)
	}

	// Process each /proc/PID (PID starts with a digit).

	for _, f := range procFiles {
		if f.Name()[0] >= '1' && f.Name()[0] <= '9' {
			for _, nsFile := range namespaces {
				nsi.addProcessNamespace(f.Name(),
					nsFile, opts, false)
			}
		}
	}
}

// printAllPIDsFor() looks up the 'NStgid' field in the /proc/PID/status
// file of 'pid' and displays the set of PIDs contained in that field.

func printAllPIDsFor(pid int, opts CmdLineOptions) {

	sfile := "/proc/" + strconv.Itoa(pid) + "/status"

	file, err := os.Open(sfile)
	if err != nil {

		// Probably, the process terminated between the time we
		// accessed the namespace files and the time we tried to open
		// /proc/PID/status. We print a diagnostic message and keep
		// going.

		fmt.Print("[can't open " + sfile + "]")
		return
	}

	defer file.Close() // Close file on return from this function.

	// Scan file line by line, looking for 'NStgid:' entry (not the
	// misnamed 'NSpid' field!), and print the corresponding set of PIDs.

	re := regexp.MustCompile(":[ \t]*")

	s := bufio.NewScanner(file)
	for s.Scan() {
		match, _ := regexp.MatchString("^NStgid:", s.Text())
		if match {
			tokens := re.Split(s.Text(), -1)

			if opts.useColor {
				fmt.Print(PID_COLOR)
			}
			fmt.Print("{ ", tokens[1], " }")
			if opts.useColor {
				fmt.Print(NORMAL)
			}

			break
		}
	}
}

// Print a sorted list of the PIDs that are members of a namespace.

func displayMemberPIDs(indent string, pids []int, opts CmdLineOptions) {

	// If the namespace has no member PIDs, there's nothing to do. (This
	// could happen if a parent namespace has no member processes, but has
	// a child namespace that has a member process.)

	if len(pids) == 0 {
		return
	}

	sort.Ints(pids)

	if opts.showCommand || opts.showAllPids {
		displayPIDsOnePerLine(indent, pids, opts)
	} else {
		displayPIDsAsList(indent, pids, opts)
	}
}

// displayPIDsOnePerLine() prints 'pids' in sorted order, one per line,
// optionally with the name of the command being run by the process.  This
// function is called because either 'opts.showCommand' or 'opts.showAllPids'
// was true.

func displayPIDsOnePerLine(indent string, pids []int, opts CmdLineOptions) {

	for _, pid := range pids {

		fmt.Print(indent + strings.Repeat(" ", 8))

		// If the "--show-all-pids" option was specified (which means
		// that "--pidns" must also have been specified), then print
		// all of the process's PIDs in each of the PID namespaces of
		// which it is a member. Otherwise, print the PID in the
		// current PID namespace.

		if opts.showAllPids {
			printAllPIDsFor(pid, opts)

			if !opts.showCommand {
				fmt.Println()
			}

		} else { // 'opts.showCommand' must be true

			if opts.useColor {
				fmt.Print(PID_COLOR)
			}
			fmt.Printf("%-5d", pid)
			if opts.useColor {
				fmt.Print(NORMAL)
			}
		}

		if opts.showCommand {

			// Print the command being run by the process.

			commFile := "/proc/" + strconv.Itoa(pid) + "/comm"

			buf, err := ioutil.ReadFile(commFile)
			if err != nil {

				// Probably, the process terminated between the
				// time we accessed the namespace files and the
				// time we tried to open /proc/PID/comm.

				fmt.Println("[can't open " + commFile + "]")
			} else {
				fmt.Print("  " + string(buf))
			}
		}
	}
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

func wrapText(text string, width int, indent string) string {

	// Break up text on white space to produce a slice of words

	words := strings.Fields(text)

	// If there were no words, return an empty string.

	if len(words) == 0 {
		return ""
	}

	result := indent + words[0]
	col := len(words[0])

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

// displayPIDsAsList() prints the PIDs in 'pids' as a sorted list, with
// multiple PIDs per line. We produce a list of PIDs that is suitably wrapped
// and indented, rather than a long single-line list.  The output is targeted
// for the terminal width, but even when deeply indenting, a minimum number of
// characters is displayed on each line.

func displayPIDsAsList(indent string, pids []int, opts CmdLineOptions) {

	// Even if deeply indenting, always display at least 'minDisplayWidth'
	// characters on each line.

	const minDisplayWidth = 32

	totalIndent := indent + strings.Repeat(" ", 8)

	outputWidth := getTerminalWidth() - len(totalIndent)
	if outputWidth < minDisplayWidth {
		outputWidth = minDisplayWidth
	}

	// Convert slice of ints to a string of space-delimited words

	res := "[ " + strconv.Itoa(pids[0])
	for _, pid := range pids[1:] {
		res += " " + strconv.Itoa(pid)
	}
	res += " ]"

	if opts.useColor {
		fmt.Print(PID_COLOR)
	}

	res = wrapText(res, outputWidth, totalIndent)
	if opts.useColor {
		res = colorEachLine(res, PID_COLOR)
	}

	fmt.Println(res)
}

// displayNamespaceTree() recursively displays the namespace subtree inside
// 'nsi.nsList' that is rooted at 'ns'.

func (nsi *NamespaceInfo) displayNamespaceTree(ns NamespaceID, level int,
	opts CmdLineOptions) {

	// Display 'ns' if its type is one of those specified in
	// 'opts.namespaces', but always display user namespaces.

	if nsi.nsList[ns].nsType == CLONE_NEWUSER ||
		nsi.nsList[ns].nsType&opts.namespaces != 0 {

		nsi.displayNamespace(ns, level, opts)
	}

	// Recursively display the child namespaces.

	for _, child := range nsi.nsList[ns].children {
		nsi.displayNamespaceTree(child, level+1, opts)
	}
}

// Display the namespace node with the key 'ns'. 'level' is our current level
// in the tree, and is used to produce suitably indented output.

func (nsi *NamespaceInfo) displayNamespace(ns NamespaceID, level int,
	opts CmdLineOptions) {

	indent := strings.Repeat(" ", level*4)

	// Display the namespace type and ID (device ID + inode number).

	colorUserNS := nsi.nsList[ns].nsType == CLONE_NEWUSER && opts.useColor

	if colorUserNS {
		fmt.Print(USERNS_COLOR)
	}

	if ns == invisUserNS {
		fmt.Println("[invisible ancestor user NS]")
	} else {
		fmt.Print(indent+namespaceMap[nsi.nsList[ns].nsType]+" ", ns)

		// For user namespaces, display creator UID.

		if nsi.nsList[ns].nsType == CLONE_NEWUSER {
			fmt.Print(" <UID: ", nsi.nsList[ns].creatorUID)
			if len(flag.Args()) == 0 {
				fmt.Print(";  ")
				fmt.Print("u: ", nsi.nsList[ns].uidMap, ";   ")
				fmt.Print("g: ", nsi.nsList[ns].gidMap)
			}
			fmt.Print(">")
		}

		fmt.Println()
	}

	if colorUserNS {
		fmt.Print(NORMAL)
	}

	// Optionally display member PIDs for the namespace.

	if opts.showPids {
		displayMemberPIDs(indent, nsi.nsList[ns].pids, opts)
	}
}

// displayNamespaceHierarchies() displays the namespace hierarchy/hierarchies
// specified by the command-line options.

func (nsi *NamespaceInfo) displayNamespaceHierarchies(opts CmdLineOptions) {

	if opts.subtreePID == "" { // No "--subtree" option was specified

		// Display the namespace tree rooted at the initial namespace.

		nsi.displayNamespaceTree(nsi.rootNS, 0, opts)

		// Display the namespaces owned by (invisible) ancestor user
		// namespaces.

		if _, fnd := nsi.nsList[invisUserNS]; fnd {
			nsi.displayNamespaceTree(invisUserNS, 0, opts)
		}

	} else {

		// Display subtree of the namespace hierarchy rooted at the
		// namespace of the PID specified in the "--subtree" option.

		nsFile := "user"
		if opts.showPidnsHierarchy {
			nsFile = "pid"
		}

		namespaceFD := openNamespaceSymlink(opts.subtreePID, nsFile)

		nsi.displayNamespaceTree(newNamespaceID(namespaceFD), 0, opts)

		syscall.Close(namespaceFD)
	}
}

// openNamespaceSymlink() opens a user or PID namespace symlink (specified in
// 'nsFile') for the process with the specified 'pid' and returns the resulting
// file descriptor.

func openNamespaceSymlink(pid string, nsFile string) int {

	symlinkPath := "/proc/" + pid + "/ns/" + nsFile

	namespaceFD, err := syscall.Open(symlinkPath, syscall.O_RDONLY, 0)

	if namespaceFD < 0 {
		fmt.Println("Error finding namespace subtree for PID"+
			pid+":", err)
		os.Exit(1)
	}

	return namespaceFD
}

// showUsageAndExit() prints a command-line usage message for this program and
// terminates the program with the specified 'status' value.

func showUsageAndExit(status int) {
	fmt.Println(
		`Usage: namespaces_of [options] [--subtree=<pid> | <pid>...]

Show the namespace memberships of one or more processes in the context of the
user or PID namespace hierarchy.

This program does one of the following:
* If provided with one or more PID command-line arguments, the program shows
  the namespace memberships of those processes.
* Otherwise, if the '--subtree=<pid>' option is specified, then the program
  shows the subtree of the user or PID namespace hierarchy that is rooted at
  the namespace of the specified PID.
* Otherwise, the program shows the namespace memberships of all processes on
  the system.

By default, the program shows namespace memberships in the context of the user
namespace hierarchy, showing also the nonuser namespaces owned by each user
namespace. If the '--pidns' option is specified, the program shows only
the PID namespace hierarchy, omitting other types of namespace.

Options:

--all-pids	For each displayed process, show PIDs in all namespaces of
		which the process is a member (used only in conjunction with
		'--pidns').
--namespaces=<list>
		Show just the listed namespace types when displaying the
		user namespace hierarchy. <list> is a comma-separated list
		containing one or more of "cgroup", "ipc", "mnt", "net", "pid",
		"time", "user", and "uts". (The default is to include all
		nonuser namespace types in the display of the user namespace
		hierarchy.) To see just the user namespace hierarchy, use
		"--namespaces=user".
--no-color	Suppress the use of color in the displayed output.
--no-pids	Suppress the display of the processes that are members
		of each namespace.
--pidns         Display the PID namespace hierarchy (rather than the user
		namespace hierarchy).
--show-comm	Displays the command being run by each process.

Syntax notes:
* No PID command-line arguments may be supplied when using '--subtree=<pid>'.
* At most one of '--namespaces' and '--pidns' may be specified.
* '--all-pids' can be specified only in conjunction with '--pidns'.
* '--no-pids' can't be specified in conjunction with either '--show-comm'
  or '--all-pids'.`)

	os.Exit(status)
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
		"Don't show PIDs that are members of each namespace")
	showCommandPtr := flag.Bool("show-comm", false,
		"Show command run by each PID")
	allPidsPtr := flag.Bool("all-pids", false,
		"Show all PIDs of each process")
	pidnsPtr := flag.Bool("pidns", false, "Show PID "+
		"namespace hierarchy (instead of user namespace hierarchy)")
	subtreePtr := flag.String("subtree", "", "Show namespace subtree "+
		"rooted at namespace of specified process")
	namespacesPtr := flag.String("namespaces", "", "Show just the "+
		"specified namespaces")

	flag.Parse()

	opts.useColor = !*noColorPtr
	opts.showPids = !*noPidsPtr
	opts.showPidnsHierarchy = *pidnsPtr
	opts.showCommand = *showCommandPtr
	opts.showAllPids = *allPidsPtr
	opts.subtreePID = *subtreePtr

	if *helpPtr {
		showUsageAndExit(0)
	}

	if *namespacesPtr != "" && opts.showPidnsHierarchy {
		fmt.Println("'--namespaces=<list>' can't be specified " +
			"with '--pidns'")
		showUsageAndExit(1)
	}

	if opts.showAllPids && !opts.showPidnsHierarchy {
		fmt.Println("'--all-pids' can be specified only with '--pidns'")
		showUsageAndExit(1)
	}

	if !opts.showPids && (opts.showCommand || opts.showAllPids) {
		fmt.Println("'--no-pids' can't be combined with " +
			"'--show-comm' or '--all-pids'")
		showUsageAndExit(1)
	}

	if opts.subtreePID != "" && len(flag.Args()) > 0 {
		fmt.Println("No PID arguments may specified in combination " +
			"with the '--subtree=<pid>' option")
		showUsageAndExit(1)
	}

	// If "--namespaces=<list>" was specified, parse list of namespaces
	// to display, by tokenizing <list> on comma delimiters, finding each
	// token string in 'namespaceMap', and adding corresponding key
	// (a CLONE_NEW* value) to 'opts.namespaces'.

	list := namespaceSymlinkNames // Default is all available namespaces

	if *namespacesPtr != "" {
		list = strings.Split(*namespacesPtr, ",")
	}

	opts.namespaces = 0

	for _, nsName := range list {
		nsFlag := 0
		for k, v := range namespaceMap {
			if v == nsName {
				nsFlag = k
			}
		}

		if nsFlag == 0 {
			fmt.Println("Bad namespace for --namespaces " +
				"option: " + nsName)
			showUsageAndExit(1)
		}

		opts.namespaces |= nsFlag
	}

	return opts
}

// Read the contents of the UID or GID map of the process with the specified
// 'pid'. 'mapName' is either "uid_map" or "gid_map". The returned string
// contains the map with white space compressed.

func readMap(pid int, mapName string) (bool, string) {

	mapFile := "/proc/" + strconv.Itoa(pid) + "/" + mapName

	buf, err := ioutil.ReadFile(mapFile)
	if err != nil {

		// Probably, the process terminated between the
		// time we accessed the namespace files and the
		// time we tried to open the map file.

		return false, "deleted"
	} else {
		space := regexp.MustCompile(`\s+`)
		return true, space.ReplaceAllString(strings.TrimSpace(string(buf)), " ")
	}

}

// Add UID and GID maps for all of the user namespaces in 'nsi'

func (nsi *NamespaceInfo) addUidGidPMaps() {

	for _, ns := range nsi.nsList {
		if ns.nsType == CLONE_NEWUSER {
			ns.uidMap = "deleted"
			ns.gidMap = "deleted"

			// Walk through the list of PIDs in the namespace
			// until we can successfully read the contents
			// of a /proc/PID/[ug]id_map file. (We try all
			// PIDs in the list because some PIDs may have
			// terminated already.)

			for _, pid := range ns.pids {
				fnd, val := readMap(pid, "uid_map")
				if fnd {
					ns.uidMap = val
					break
				}
			}

			for _, pid := range ns.pids {
				fnd, val := readMap(pid, "gid_map")
				if fnd {
					ns.gidMap = val
					break
				}
			}
		}
	}
}

func main() {

	var nsi = NamespaceInfo{nsList: make(NamespaceList)}

	var opts CmdLineOptions = parseCmdLineOptions()

	// Determine which namespace symlink files are to be processed.
	// (By default, all namespaces are processed, but this can be
	// changed via command-line options.)

	nsSymlinks := namespaceSymlinkNames
	if opts.showPidnsHierarchy {
		nsSymlinks = []string{"pid"}
	}

	// Add namespace entries for specified processes.

	if len(flag.Args()) == 0 || opts.subtreePID != "" {
		nsi.addNamespacesForAllProcesses(nsSymlinks, opts)

		// If we scanned all processes on the system (i.e., no PID
		// command-line arguments were supplied), then we probably have
		// at least one PID in each user namespace. This enables us to
		// discover the UID and GID map for each user namespace, so do
		// that discovery in order that we can display the maps.  We
		// don't do this if only some of the PIDs on the system are
		// scanned, since then it's likely that we gathered information
		// about some user namespaces without discovering any of their
		// member processes.

		if len(flag.Args()) == 0 {
			nsi.addUidGidPMaps()
		}

	} else {

		// Add namespaces for PIDs named in the command-line arguments.
		// (flag.Args() is the set of command-line words that were
		// not options.)

		for _, pid := range flag.Args() {
			for _, nsFile := range nsSymlinks {
				nsi.addProcessNamespace(pid, nsFile, opts, true)
			}
		}
	}

	// Display the results of the namespace scan.

	nsi.displayNamespaceHierarchies(opts)
}
