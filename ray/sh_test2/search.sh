#!/bin/bash
#First you can use grep (-n) to find the number of lines of string.
#Then you can use awk to separate the answer.
touch $3
rm $3
touch $3
touch temp
grep -n $2 $1 > temp
awk -F: '{print $1}' temp >> $3
rm temp
