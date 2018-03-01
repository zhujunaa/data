#!/bin/bash

fname=$(basename $1)


if [ -f $1 ]; then
	iconv -f gb18030 -t utf8 $1 -o /dev/null &> /dev/null
	if [ $? = 0 ]; then
		iconv -f gb18030 -t utf8 $1 -o utf8/$fname
	else
		copy -f $1 -o utf8/$fname
	fi
else

	s=${1%/}
	b=${s##*/}
	if [ $b = "." ]; then
		b="/tmp/utf8"
	elif [ $b = ".." ]; then
		b="/tmp/utf8"
	else
		echo  ""
	fi

	if [ ! -d $b ]; then
		mkdir -p $b
	fi
	
	find $1 -type d -exec ./dir.sh {} $1 \;
	find $1 -type f -exec ./file.sh {} $1 \;
fi

