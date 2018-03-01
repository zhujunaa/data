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
	
fname=$d
iconv -t gb18030 -f utf8 $1 -o /dev/null &> /dev/null
if [ $? = 0 ]; then
	echo "iconv $1 to $fname as code gbk"
	iconv  -t gb18030 -f utf8 $1 -o $fname 
else
	echo "do copy $1"
	cp -f $1 $fname
fi
