#!/bin/bash
sed "s/?//g" $1 > input.txt
grep "legal" input.txt
x=$?
if [ $x -eq 0 ]
then
echo -e "yes\n" >> input.txt
a=0
i=1
for s in $(cat input.txt)
do
	while [ $i -le $2 ]
	do 
		if [ "1" == ${s:${a}:1} ]
		then
			echo $i >> input.txt
           break
		fi
		let i++
		let a++
	done
break
done
fi
