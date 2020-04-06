#!/bin/bash
set -x
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

