#!/bin/bash

set -e
BIN=/usr/sbin/pam_handshake_server
PID_FILE=/var/pam_handshake.pid
LOG_FILE=/var/log/pam_handshake.log
SOCKET=/var/pam_handshake.socket
STACK=irods

if [ -e "$PID_FILE" ]
then
    pid=$( cat "$PID_FILE" )
    set +e
    ps -p $pid -o pid=
    ret=$?
    set -e
    if [ $ret = "0" ]
    then
        kill $pid
    else
        echo "process not running"
    fi
else
    echo "process not running"
fi

msg="starting $BIN  --socket --verbose --addr $SOCKET --stack $STACK"
echo $msg
echo $msg >> $LOG_FILE
$BIN  --socket --verbose --addr $SOCKET --stack $STACK 2>&1 >> $LOG_FILE &
pid=$!
echo "started with pid $pid"
echo $pid > $PID_FILE

