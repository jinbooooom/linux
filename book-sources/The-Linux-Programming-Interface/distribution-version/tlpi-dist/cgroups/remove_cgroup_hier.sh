#!/bin/sh
#
# Usage: sh remove_cgroup_hier.sh <cgroup-v1-mount-point>
#
# In preparation for a clean unmount of the specified cgroup hierarchy,
# move all processes to the root cgroup, and remove all child cgroups.
# The main use for doing this is to ensure that a cgroup v1 directory
# is properly unmounted so that the corresponding controller becomes
# available in the cgroups v2 unified hierarchy.

cd $1

# Generate a list of the cgroup.procs files in the subdirectories

cgroup_procs_files=$(find */ -name cgroup.procs 2> /dev/null)

# Maybe there were no directories (or, the given pathname was not actually
# a cgroup v1 mount point). In that case, we have nothing to do.

if test $(echo $cgroup_procs_files | wc -w) -eq 0; then
	echo "Nothing to be done!"
	exit
fi

# Read the cgroup.procs files to get a list of PIDs

pids=$(cat $cgroup_procs_files | sort)

# Move each PID to the root cgroup ($1/cgroup.procs)

for p in $pids ; do
	echo $p > cgroup.procs 2> /dev/null

	if test $? -ne 0; then
		echo -n "Error moving PID $p to root cgroup"
		if test $(ps $p | wc -l) -lt 2; then
			echo -n " (process appears to have already terminated)"
		fi
		echo
	fi
done

# Remove all of the child cgroup directories. This needs to be done in a
# bottom-up fashion, so we reverse sort the set of directory pathnames.

rmdir $(find */ -type d | sort -r)
