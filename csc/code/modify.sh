#!/bin/bash
#replace
#sed -i  's/'${2}'/'${3}'/g' $1
sed -i "s/{$2}/{$3}/g" $1
