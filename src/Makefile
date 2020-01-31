CPP=g++ 
CPP_FLAGS=-std=c++11 -g
OBJ=	session.o \
	pam_conversation.o \
	http_header.o \
	connection.o \
	server.o

LIBS=-lpam -lpthread
all: ../server ../auth_check ../libpam_http_server.a

../server: ${OBJ} server_main.o
	${CPP} ${CPP_FLAGS} ${OBJ} server_main.o -o ../server ${LIBS}

../auth_check: ${OBJ} auth_check_main.o
	${CPP} ${CPP_FLAGS} ${OBJ} auth_check_main.o -o ../auth_check ${LIBS}

../libpam_http_server.a: ${OBJ}
	ar rcs $@ ${OBJ}

session.o: session.h server.h ipam_client.h session.cpp
	${CPP} ${CPP_FLAGS} -c session.cpp -o session.o

pam_conversation.o: pam_conversation.cpp pam_conversation.h ipam_client.h
	${CPP} ${CPP_FLAGS} -c pam_conversation.cpp -o pam_conversation.o

http_header.o: http_header.h http_header.cpp
	${CPP} ${CPP_FLAGS} -c http_header.cpp -o http_header.o

connection.o: connection.h server.h connection.cpp
	${CPP} ${CPP_FLAGS} -c connection.cpp -o connection.o

server.o: server.h connection.h http_header.h session.h server.cpp
	${CPP} ${CPP_FLAGS} -c server.cpp -o server.o

server_main.o: server_main.cpp
	${CPP} ${CPP_FLAGS} -c server_main.cpp -o server_main.o

auth_check_main.o: auth_check_main.cpp
	${CPP} ${CPP_FLAGS} -c auth_check_main.cpp -o auth_check_main.o

clean:
	rm -f *.o
	rm -f ../server
	rm -f ../libpam_http_server.a

