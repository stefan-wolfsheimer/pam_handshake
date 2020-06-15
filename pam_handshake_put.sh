#!/bin/bash
_SESSION=$1
_DATA=$2

if [ ! -z ${_DATA} ]
then
    curl --unix-socket /var/pam_handshake.socket -X PUT http://local/${_SESSION} -d ${_DATA}
else
    curl --unix-socket /var/pam_handshake.socket -X PUT http://local/${_SESSION}
fi
