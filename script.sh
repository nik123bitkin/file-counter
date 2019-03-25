#!/bin/bash
truncate -s 0 $2

exec 2>errlog

dirs=$(find $1 -type d -printf "%p\n")

for dir in $dirs
do
	files=$(find $dir -maxdepth 1 -type f -printf "%s\n")
	fsize_sum=0
	fcount=0
	max=-1
	for file in $files
	do
		fcount=$(($fcount+1))
		fsize_sum=$(($fsize_sum + $file))
		if [ $file -gt $max ]
		then
			max=$file
		fi
	done

	echo $(realpath $dir) $fcount $fsize_sum $max >> $2
done
