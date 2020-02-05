# Build binaries locally

```
make 
make test
```

# Build the binaries in the functional test container

```
cd functional_test
make
```

# Usage of the Server application

## Usage of standalone PAM conversation application

Create pam stack configuration file, e.g.
```
cat /etc/pam.d/allowall
# CentOS
# auth required pam_exec.so debug stdout /usr/bin/echo test

# Ubuntu 
auth required pam_exec.so debug stdout /bin/echo test

auth sufficient pam_allow.so
```

Run the check
```
./auth_check --name allowall
```

Other examples:
[functional_test/pam.d](pam.d)


## Using the HTTP server
**Listening on Unix Domain Socket**

```
./server --socket --verbose --addr /path/to/socket/file/server.socket
```

Using curl to communicate with the server:

```
curl --unix-socket /path/to/socket/file/server.socket -X GET http://local/somepath
```


**Listening on port**

```
./server --verbose --addr 0.0.0.0 --port 8080
```

**Using test container**

```
docker run -p 8080:8080 pam_handshake_test
```

# Software design

![StateDiagram](doc/StateDiagram.png)

# Example session

## Configure pam:

Create PAM db:
```
echo -e "mara\npw" | db_load -t hash -T /etc/pam.d/irods_user.db
```

Configure PAM stack:

vi /etc/pam.d/irods
```
auth required pam_exec.so debug stdout /bin/echo test
auth required pam_userdb.so db=/etc/pam.d/irods_user crypt=none debug dump stdout
```

Start the server
```
./server --socket --verbose --addr server.socket
```

Communicate with the server

## Open new session

```
SESSION=$( curl --unix-socket server.socket -X POST http://local/ )
```

Get the state of the session
```
curl --unix-socket /path/to/socket/file/server.socket -X GET http://local/$SESSION
RUNNING
```

## Transition to the next state

```
curl  --unix-socket  server.socket -X PUT http://local/$SESSION
```

Check the state
```
curl --unix-socket /path/to/socket/file/server.socket -X GET http://local/$SESSION
NEXT
```
Session underwent the transitions: (RUNNING -> READY -> NEXT)

```
curl  --unix-socket  server.socket -X PUT http://local/$SESSION
```

## Transition to the WAITING state

```
curl  --unix-socket  server.socket -X PUT http://local/$SESSION
WAITING
login:
```

Check the state
```
curl --unix-socket server.socket -X GET http://local/$SESSION
WAITING
```
Session is waiting for answer

## Giving an answer

```
curl  --unix-socket  server.socket -X PUT http://local/$SESSION -d 'mara'
```

Check the state
```
curl --unix-socket server.socket -X GET http://local/$SESSION
NEXT
```

## Transition to the WAITING state
```
curl  --unix-socket  server.socket -X PUT http://local/$SESSION
WAITING
```

## Giving the password answer

```
curl  --unix-socket  server.socket -X PUT http://local/$SESSION -d 'pw'
```

## Going to the next state
```
curl  --unix-socket  server.socket -X PUT http://local/$SESSION
AUTHROIZED
```








