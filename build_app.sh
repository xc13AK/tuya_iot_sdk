#!/bin/sh

APP_BIN_NAME=
if [ -z "$1" ];then
	echo "please input the target app name "
	exit 1
else
	APP_BIN_NAME=$1
fi

if [ -z "$2" ];then
    echo "please input the app sw version(for example:the format is "1.1.1")!!!"
    exit 1
else
    USER_SW_VER=$2
fi


#开始编译APP
if [ -z "$3" ];then
    make  APP_BIN_NAME=$APP_BIN_NAME USER_SW_VER=$USER_SW_VER build_app
else
    make  APP_BIN_NAME=$APP_BIN_NAME USER_SW_VER=$USER_SW_VER COMPILE_PREX=$3 build_app
fi
