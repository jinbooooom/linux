/* pid_namespaces.go

   Show the PID namespace hierarchy.

   The (rather more complicated) namespaces_of.go program provides a superset
   of the functionality of this program.

   Copyright (C) Michael Kerrisk, 2018

   Licensed under GNU General Public License version 3 or later
*/

package main

import (
	"bufio"
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

// A namespace is uniquely identified by the combination of a device ID
// and an inode number.

type NamespaceID struct {
	device    uint64 // dev_t
	inode_num uint64 // ino_t
}

// For each namespace, we record the child namespaces and the member processes.

type NamespaceAttribs struct {
	children []NamespaceID // Child namespaces
	pids     []int         // Member processes
}

// The following map records all of the namespaces that we visit.

var NSList = make(map[NamespaceID]*NamespaceAttribs)

// While adding the first namespace to 'NSList', we'll discover the
// ancestor of all PID namespaces (the root of the PID namespace
// hierarchy).  We record that namespace in 'initialPidNS'.

var initialPidNS NamespaceID

// Create and return a new namespace ID using the device ID and inode
// number of the namespace referred to by 'namespaceFD'.

func NewNamespaceID(namespaceFD int) NamespaceID {
	var sb syscall.Stat_t
	var err error

	// Obtain the device ID and inode number of the namespace file.
	// These values together form the key for the 'NSList' map entry.

	err = syscall.Fstat(namespaceFD, &sb)
	if err != nil {
		fmt.Println("syscall.Fstat():", err)
		os.Exit(1)
	}

	return NamespaceID{sb.Dev, sb.Ino}
}

// AddNamespace() adds the namespace referred to by the file descriptor
// 'namespaceFD' to the 'NSList' map (creating an entry in the map if one does
// not already exist) and optionally adds the PID specified in 'pid' to the
// list of PIDs that are resident in that namespace.
//
// This function is recursive, calling itself to ensure that an entry is also
// created for the parent PID namespace of the namespace referred to by
// 'namespaceFD'.  Once that has been done, the namespace referred to by
// 'namespaceFD' is made a child of the parent namespace. (Note that, except
// in the case of the initial PID namespace, a parent namespace must exist,
// since it is pinned into existence by the existence of the child namespace
// (and that namespace is in turn pinned into existence by the open file
// descriptor 'namespaceFD').
//
// 'pid' is a PID to be added to the list of PIDs resident in this namespace.
// When called recursively to create ancestor namespace entries, this function
// is called with 'pid' as -1, meaning that no PID needs to be added for this
// namespace entry.
//
// The return value of the function is the ID of the namespace entry (i.e.,
// the device ID and inode number corresponding to the user namespace file
// referred to by 'namespaceFD').

func AddNamespace(namespaceFD int, pid int) NamespaceID {

	const NS_GET_PARENT = 0xb702 // ioctl() to get parent namespace

	nsid := NewNamespaceID(namespaceFD)

	if _, fnd := NSList[nsid]; !fnd {

		// Namespace entry does not yet exist in 'NSList' map;
		// create it.

		NSList[nsid] = new(NamespaceAttribs)

		// Get a file descriptor for the parent namespace.

		ret, _, err := syscall.Syscall(syscall.SYS_IOCTL,
			uintptr(namespaceFD), uintptr(NS_GET_PARENT), 0)
		parentFD := (int)((uintptr)(unsafe.Pointer(ret)))

		if parentFD == -1 && err == syscall.EPERM {

			// If NS_GET_PARENT failed with EPERM (meaning no
			// visible parent), then this is the root PID
			// namespace (or, at least, the topmost visible
			// PID namespace); remember it.

			initialPidNS = nsid

		} else if parentFD == -1 {

			fmt.Println("ioctl(NS_GET_PARENT):", err)
			os.Exit(1)

		} else {

			// We have a parent namespace; make sure it
			// has an entry in the map. Don't record the
			// process as being a member of that namespace.

			p := AddNamespace(parentFD, -1)

			// Make the current namespace entry ('nsid') a child
			// of the parent/owning user namespace entry.

			NSList[p].children = append(NSList[p].children, nsid)

			syscall.Close(parentFD)
		}
	}

	// Add PID to PID list for this namespace entry.

	if pid > 0 {
		NSList[nsid].pids = append(NSList[nsid].pids, pid)
	}

	return nsid
}

// AddProcessNamespace() processes a single /proc/PID/ns/pid entry, creating
// a namespace entry for that file and, as necessary, namespace entries for
// all ancestor namespaces going back to the initial PID namespace.  'pid'
// is a string containing a PID.

func AddProcessNamespace(pid string) {

	// Obtain a file descriptor that refers to the PID namespace
	// corresponding to 'pid'.

	namespaceFD, err := syscall.Open("/proc/"+pid+"/ns/pid",
		syscall.O_RDONLY, 0)

	if namespaceFD < 0 {
		fmt.Println("open("+"/proc/"+pid+"/ns/pid):", err)
		os.Exit(1)
	}

	// Add namespace entry for this namespace, and all of its ancestor
	// PID namespaces.

	npid, _ := strconv.Atoi(pid)
	AddNamespace(namespaceFD, npid)

	syscall.Close(namespaceFD)
}

// PrintAllPIDsFor() looks up the 'NStgid' field in the /proc/PID/status
// file of 'pid' and displays the set of PIDs contained in that field

func PrintAllPIDsFor(pid int) {

	sfile := "/proc/" + strconv.Itoa(pid) + "/status"

	file, err := os.Open(sfile)
	if err != nil {
		// Probably, the process terminated between the time we
		// accessed the namespace files and the time we tried to
		// open /proc/PID/status.
		fmt.Print("[can't open " + sfile + "]")
		return
	}

	defer file.Close() // Close file on return from this function.

	re := regexp.MustCompile(":[ \t]*")

	// Scan file line by line, looking for 'NStgid:' entry, and print
	// corresponding set of PIDs.

	s := bufio.NewScanner(file)
	for s.Scan() {
		match, _ := regexp.MatchString("^NStgid:", s.Text())
		if match {
			tokens := re.Split(s.Text(), -1)
			fmt.Print("[", tokens[1], "]  ")

			break
		}
	}
}

// Print a sorted list of the PIDs that are members of a namespace.

func PrintMemberPIDs(indent string, pids []int) {

	sort.Ints(pids)

	for _, pid := range pids {
		fmt.Print(indent + "        ")
		PrintAllPIDsFor(pid)
		fmt.Println()
	}
}

// DisplayNamespaceTree() recursively displays the namespace tree rooted at
// 'nsid'. 'level' is our current level in the tree, and is used to produce
// suitably indented output.

func DisplayNamespaceTree(nsid NamespaceID, level int) {

	indent := strings.Repeat(" ", level*4)

	fmt.Println(indent, nsid)

	PrintMemberPIDs(indent, NSList[nsid].pids)

	for _, child := range NSList[nsid].children {
		DisplayNamespaceTree(child, level+1)
	}
}

func main() {

	// Fetch a list of the filenames under /proc.

	files, err := ioutil.ReadDir("/proc")
	if err != nil {
		fmt.Println("ioutil.Readdir():", err)
		os.Exit(1)
	}

	// Process each /proc/PID (PID starts with a digit).

	for _, f := range files {
		if f.Name()[0] >= '1' && f.Name()[0] <= '9' {
			AddProcessNamespace(f.Name())
		}
	}

	// Display the namespace tree rooted at the initial PID namespace.

	DisplayNamespaceTree(initialPidNS, 0)
}
