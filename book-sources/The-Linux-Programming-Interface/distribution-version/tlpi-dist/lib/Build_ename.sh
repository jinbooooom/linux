#!/bin/sh
#
# Create a new version of the file ename.c.inc by parsing symbolic
# error names defined in errno.h
#
echo '#include <errno.h>' | cpp -dM | 
sed -n -e '/#define  *E/s/#define  *//p' |sort -k2n |
awk '
BEGIN {
        entries_per_line = 4
        line_len = 68;
        last = 0;
        varname ="    enames";
        print "static char *ename[] = {";
        line =  "    /*   0 */ \"\"";
}
 
{
    if ($2 ~ /^E[A-Z0-9]*$/) {      # These entries are sorted at top
        synonym[$1] = $2;
    } else {
        while (last + 1 < $2) {
            last++;
            line = line ", ";
            if (length(line ename) > line_len || last == 1) {
                print line;
                line = "    /* " last " */ ";
                line = sprintf("    /* %3d */ ", last);
            }
            line = line "\"" "\"" ;
        }
        last = $2;
        ename = $1;
        for (k in synonym)
            if (synonym[k] == $1) ename = ename "/" k;
 
            line = line ", ";
            if (length(line ename) > line_len || last == 1) {
                print line;
                line = "    /* " last " */ ";
                line = sprintf("    /* %3d */ ", last);;
            }
            line = line "\"" ename "\"" ;
    }
}
END {
    print  line;
    print "};"
    print "";
    print "#define MAX_ENAME " last;
}
'
 
