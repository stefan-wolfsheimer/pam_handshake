#!/bin/bash
_SESSION=$1
curl --unix-socket /var/pam_handshake.socket -X DELETE http://local/${_SESSION}
