#!/bin/bash

#echo $1
#echo $2

btarget="$1"/"$2"

#echo $btarget

if [ -d $btarget ]; then
echo $btarget "already exists"
else
(
cd $1
./build_mingw.sh
)
fi
