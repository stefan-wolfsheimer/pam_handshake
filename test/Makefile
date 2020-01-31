CPP=g++
CPP_FLAGS=-std=c++11 -g
INCLUDE=-I/opt/include -I../src
LIBS=../libpam_http_server.a -lpam
SRC=	test_pam.cpp \
	test_http_header.cpp

all: ../runtest

../runtest: runtest.cpp ${SRC}
	${CPP} ${CPP_FLAGS} ${INCLUDE} ${SRC} runtest.cpp ${LIBS} -o ../runtest

clean:
	rm -f ../runtest
