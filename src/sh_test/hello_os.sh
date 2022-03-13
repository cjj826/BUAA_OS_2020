#!/bin/bash
#make a new dir
sed -e '8p' -e '32p' -e '128p' -e '512p' -e '1024p' -n $1 >$2
