CPP=g++ -std=c++14 -g

all:runtest pam_auth_check
	echo "ok"

pam_conversation.o: pam_conversation.cpp pam_conversation.h ipam_client.h
	${CPP} -c pam_conversation.cpp -o pam_conversation.o

pam_auth_check: pam_auth_check.cpp pam_conversation.h ipam_client.h pam_conversation.o
	${CPP} -I. pam_auth_check.cpp pam_conversation.o -lpam  -o pam_auth_check

runtest: test/runtest.cpp test/test_pam.cpp pam_conversation.o
	${CPP} -I. -I/opt/include test/runtest.cpp test/test_pam.cpp pam_conversation.o -lpam  -o runtest

clean:
	rm -f *.o
	rm -f runtest

