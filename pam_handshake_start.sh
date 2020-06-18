#!/bin/bash

set -e
BIN=/usr/sbin/pam_handshake_server
# use auth check bin because not all pam modules are thread safe
AUTH_CHECK_BIN="--conversation /usr/sbin/pam_handshake_auth_check"

PID_FILE=/var/pam_handshake.pid
LOG_FILE=/var/log/pam_handshake.log
SOCKET=/var/pam_handshake.socket
STACK=irods
GROUP=$( awk 'BEGIN{ FS="="; }{ if($1=="IRODS_SERVICE_GROUP_NAME") print $2; }' /etc/irods/service_account.config )

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

msg="starting $BIN  --socket --verbose --addr $SOCKET --stack $STACK --chgrp $GROUP $AUTH_CHECK_BIN"
echo $msg
echo $msg >> $LOG_FILE
$BIN  --socket --verbose --addr $SOCKET --stack $STACK --chgrp $GROUP $AUTH_CHECK_BIN 2>&1 >> $LOG_FILE &
pid=$!
echo "started with pid $pid"
echo $pid > $PID_FILE

