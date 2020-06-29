#!/bin/bash
curl --unix-socket /var/pam_handshake.socket -X POST http://local/
