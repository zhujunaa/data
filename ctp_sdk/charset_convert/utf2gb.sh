#!/bin/bash

fname=$(basename $1)


if [ -f $1 ]; then
	iconv -t gb18030 -f utf8 $1 -o /dev/null &> /dev/null
	if [ $? = 0 ]; then
		iconv -t gb18030 -f utf8 $1 -o gbk/$fname
	else
		copy -f $1 -o gbk/$fname
	fi
else

	s=${1%/}
	b=${s##*/}
	if [ $b = "." ]; then
		b="/tmp/gbk"
	elif [ $b = ".." ]; then
		b="/tmp/gbk"
	else
		echo  ""
	fi

	if [ ! -d $b ]; then
		mkdir -p $b
	fi
	
	find $1 -type d -exec ./dir2.sh {} $1 \;
	find $1 -type f -exec ./file2.sh {} $1 \;
fi

