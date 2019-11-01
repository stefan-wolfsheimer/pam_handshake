# pam_handshake

## build container
```
docker build -t pam_handshake .
```

## run container

```
docker run -ti -v $( pwd ):/host pam_handshake /bin/bash
```

