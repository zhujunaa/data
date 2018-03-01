#!/bin/bash

s=${2%/}
b=${s##*/}
l=${#s}+1
a=${1:l}


if [ $b = "." ]; then
	b="/tmp/gbk"
	d="$b/$a"
elif [ $b = ".." ]; then
	b="/tmp/gbk"
	d="$b/$a"
else
	d="./$b/$a"
fi
	

d=${d%/}

if [ ! -d $d ]; then
	mkdir -p $d
fi
