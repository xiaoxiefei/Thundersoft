#!/bin/bash
path=$(pwd)
echo $path
filelog=$path/message.log
TIME='['`date +%Y-%m-%d\ %H:%M:%S`']'
if [ -e $filelog ]
then
	echo "log"
	echo ${TIME} "log is create" >> $filelog
else
	echo ${TIME} "log have not create" >> $filelog

	touch $filelog
	if [ $? -eq 0 ]
	then
		echo ${TIME} "log create successfully" >> $filelog
	else
		echo ${TIME} "log create failed" >> $filelog
	fi
fi
echo ${TIME} "hello" >> message.log 

echo $?
