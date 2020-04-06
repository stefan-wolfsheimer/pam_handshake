#!/bin/bash

echo -e "mara\npw" | db_load -t hash -T /etc/pam.d/irods_user.db
echo -e "mara\npw2" | db_load -t hash -T /etc/pam.d/irods_user2.db
