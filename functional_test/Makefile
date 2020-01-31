# Build: make
# Remove image: make clean
# Run Container:
#                docker run -p 8080:8080 pam_handshake_test
#                                  or
#                docker run -e PAM_STACK_NAME=irods -p 8080:8080 pam_handshake_test

all:
	docker build -t pam_handshake_test -f Dockerfile ..


clean:
	docker image rm pam_handshake_test


