#!/usr/bin/awk -f
length > max { max = length; }
END 	     { print max; }
