#!/bin/bash

set -e
set -x

SESSION=$( curl -X POST http://127.0.0.1:8080  )
curl  -X PUT http://127.0.0.1:8080/$SESSION 
curl  -X PUT http://127.0.0.1:8080/$SESSION -d mara 
curl  -X PUT http://127.0.0.1:8080/$SESSION 
curl  -X PUT http://127.0.0.1:8080/$SESSION -d pw 
FINAL_STATE=$( curl -X PUT http://127.0.0.1:8080/$SESSION )


if [[ "$FINAL_STATE" =~ ^STATE_AUTHENTICATED.+ ]]
then
     echo "correct final state $FINAL_STATE"
     exit 0
else
     echo "wrong final state '$FINAL_STATE'"
     exit 1
fi


