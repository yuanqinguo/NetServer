#!/bin/bash

#杀死原有进程
set +e

killall SealServer || true
rm -rf ESServerLog
set -e

#启动ngixn


port=$1
thread=$2
num=$3

#启动SealServer
i=0
while [[ $i -lt $num ]] 
do
	i=$[i+1]
	setsid ./SealServer $port $thread &
	port=$[port+1]
done





