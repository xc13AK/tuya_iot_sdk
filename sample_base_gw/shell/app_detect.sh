#!/bin/sh
DETECT_TIME_S=
if [ -z "$1" ];then
	echo "please input the application"
	exit 1
fi

if [ -z "$2" ];then
	DETECT_TIME_S=2
else
	DETECT_TIME_S=$2
fi

echo "application is $1"
echo "DETECT_TIME_S is $DETECT_TIME_S"

while true
do
	pgrep "$1"
	if [ "$?" -eq 1 ];then
		$1
		echo "process has been restarted!"
	else
		echo "process already started!"
	fi
	sleep $DETECT_TIME_S
done
