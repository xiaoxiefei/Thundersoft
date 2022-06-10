#!/bin/bash

path=$(pwd)
logfile=$path/message.log
TIME='['`date +%Y-%m-%d\ %H:%M:%S`']'
index=1
for index in 1 2 3 4 5 6
do
	if [ $index -eq 6 ]
	then 
		break
	fi
	echo ${TIME} "----------Number of executions  $index----------" >> $logfile
	#判断log文件是否已经创建
	if [ -e $logfile ]
	then
		echo ${TIME} $logfile " is created." >> $logfile 
	else
		echo ${TIME} $logfile" does not exist! Attempting to create." >> $logfile

		#创建日志文件
		touch $logfile 
		if [ $? -eq 0 ]
		then
			echo ${TIME} $logfile" Create successfully" >> $logfile 
		else
			echo ${TIME} $logfile" Create failed , Replay command.sh" >> $logfile 
			continue
		fi
	fi

	#删除编译的用户程序
	rm ./User/user >>$logfile 2>&1
	echo ${TIME} "Start command.sh" >>$logfile
	
	#卸载模块
	echo ${TIME} "Attempting rmmod drive..." >>$logfile
	sudo rmmod drive >> $logfile 2>&1
	if [ $? -eq 0 ]
	then
		echo ${TIME} "Rmmod drive successfully" >>$logfile 2>&1
	else
		echo ${TIME} "Rmmod drive failed " >>$logfile 2>&1
	fi

	#执行makefile
	echo ${TIME} "Start Makefile" >>$logfile
	cd Driver/
	sudo make  >> $logfile 2>&1
	if [ $? -eq 0 ]
	then
		echo ${TIME} "Makefile successfully" >>$logfile 2>&1
	else
		echo ${TIME} "Makefile failed , Replay command.sh" >>$logfile 2>&1
		continue
	fi
	cd ..

	#加载模块
	echo ${TIME} "Start insmod drive" >> $logfile
	sudo insmod ./Driver/drive.ko >> $logfile 2>&1
	if [ $? -eq 0 ]
	then
		echo ${TIME} "insmod drive successfully" >>$logfile 2>&1
	else
		echo ${TIME} "insmod drive failed , Replay command.sh" >>$logfile 2>&1
		continue
	fi

	#编译用户程序
	echo ${TIME} "Start compiling user program" >>$logfile
	gcc ./User/user.c -o ./User/user >> $logfile 2>&1
	if [ $? -eq 0 ]
	then	
		echo ${TIME} "Compiling user program successfully" >>$logfile 2>&1
	else
		echo ${TIME} "Compiling user program failed , Replay command.sh" >>$logfile 2>&1
		continue
	fi

	#执行用户程序
	echo ${TIME} "Start user program execution" >> $logfile
	sudo ./User/user
	if [ $? -eq 0 ]
	then
		echo ${TIME} "User program execution successfully" >>$logfile 2>&1
		break
	else
		echo ${TIME} "User program execution failed , Replay command.sh" >>$logfile 2>&1
		continue
	fi
done
if [ $index -eq 6 ]
then
	echo ${TIME} "command.sh Execution failed" >>$logfile
else
	echo ${TIME} "command.sh Execution successfully" >>$logfile
fi
