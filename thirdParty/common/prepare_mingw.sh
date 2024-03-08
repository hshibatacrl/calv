#!/bin/bash

packages=("git" "make" "patch" "unzip" "python3")

for i in "${packages[@]}"
do
	which $i
	if [ "$?" = "0" ]; then
		echo $i exists
	else
		echo $i does not exist
		pacman -S $i --noconfirm
	fi
done

