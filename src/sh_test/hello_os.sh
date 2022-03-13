#!/bin/bash
#make a new dir
echo "" > $2
a=8
while [ $a -le 1024 ]
do 
	sed -n ${a}'p' $1 >> $2
	a=$[$a*4]
done
#sed -e '1p' -e '2p' -n $1 > $2
#sed -e '8p' -e '32p' -e '128p' -e '512p' -e '1024p' -n $1 > $2
