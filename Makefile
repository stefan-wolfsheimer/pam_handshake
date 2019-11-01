CPP=g++ -std=c++14 -g

all:runtest
	echo "ok"

pam_conversation.o: pam_conversation.cpp pam_conversation.h ipam_client.h
	${CPP} -c pam_conversation.cpp -o pam_conversation.o

runtest: test/runtest.cpp test/test_pam.cpp pam_conversation.o
	${CPP} -I. -I/opt/include test/runtest.cpp test/test_pam.cpp pam_conversation.o -o runtest

clean:
	rm -f *.o
	rm -f runtest

