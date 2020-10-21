#!/usr/bin/awk -f
#
# fork_whos_on_first.count.awk
#
# Copyright, Michael Kerrisk, 2002
#
# Read (on stdin) the results of running the fork_whos_on_first.c
# program to assess the percentage iof cases in which parent or child
# was first to print a post-fork() message
#
BEGIN {
    last = -1;
}

{
    # match pairs of lines by field 1 (loop counter)

    if ($1 != last) {
	cat[$2]++;
	tot++;
    }
    last = $1;
}

    # Our input file may be long - periodically print a progress 
    # message with statistics so far
NR % 200000 == 0 { 
    print "Num children = ", NR / 2;
    for (k in cat) 
 	printf "%-6s %6d %6.2f%%\n", k, cat[k], cat[k] / tot * 100;
}

END {
    print "All done";
    for (k in cat) 
 	printf "%-6s %6d %6.2f%%\n", k, cat[k], cat[k] / tot * 100;
}
