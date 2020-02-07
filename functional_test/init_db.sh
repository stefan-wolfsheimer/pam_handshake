#!/bin/bash

echo -e "mara\npw" | db_load -t hash -T /etc/pam.d/irods_user.db
