#!/bin/bash
make
touch hello_os
mkdir hello_os_dir
cp hello_os.o hello_os_dir/
cp hello_os.o hello_os_dir/hello_os
rm hello_os.o
grep "os_hello" hello_os.c | awk -F"os_hello" {'print $1'} > hello_os.txt
