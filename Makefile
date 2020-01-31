all: auth_check server runtest libpam_http_server.a

libpam_http_server.a:
	$(MAKE) -C src ../libpam_http_server.a

server:
	$(MAKE) -C src ../server

auth_check:
	$(MAKE) -C src ../auth_check

runtest: libpam_http_server.a
	$(MAKE) -C test 

test: runtest
	./runtest

clean:
	$(MAKE) -C src clean
	$(MAKE) -C test clean

