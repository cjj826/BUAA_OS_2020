#!/bin/bash
#make a new dir
touch $2
sed '8p' $1 >> $2
sed '32p' $1 >> $2
sed '128p' $1 >> $2
sed '512p' $1 >> $2
sed '1024p' $1 >> $2
