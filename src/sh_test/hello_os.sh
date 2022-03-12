#!/bin/bash
#make a new dir
touch $2
rm $2
touch $2
a=8
while [ $a -le 1024 ]
do 
	sed -n ${a}'p' $1 >> $2
	a=$[$a*4]
done
