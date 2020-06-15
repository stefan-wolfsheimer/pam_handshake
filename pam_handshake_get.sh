#!/bin/bash
_SESSION=$1
curl --unix-socket /var/pam_handshake.socket -X GET http://local/${_SESSION}
