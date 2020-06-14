LIBS2=-lpython2.7
INCLUDE2=-I/usr/include/python2.7
LIBS3=-lpython3.6m
INCLUDE3=-I/usr/include/python3.6m

GCC_FLAGS=-fPIC --shared -fno-stack-protector -Wall -Werror

# python-libs-2.7.5-88.el7.x86_64
# python-devel-2.7.5-88
ifeq ($(wildcard /usr/lib64/libpython2.7.so), /usr/lib64/libpython2.7.so)
       GCC_DEFINES2=-DLIBPYTHON_SO='"/usr/lib64/libpython2.7.so.1.0"'
endif
ifeq ($(wildcard /usr/lib64/libpython3.so), /usr/lib64/libpython3.so)
       GCC_DEFINES3=-DLIBPYTHON_SO='"/usr/lib64/libpython3.so"'
endif

ifeq ($(wildcard /usr/lib/x86_64-linux-gnu/libpython2.7.so), /usr/lib/x86_64-linux-gnu/libpython2.7.so)
	GCC_DEFINES2=-DLIBPYTHON_SO='"/usr/lib/x86_64-linux-gnu/libpython2.7.so.1.0"'
endif 
ifeq ($(wildcard /usr/lib/x86_64-linux-gnu/libpython3.6m.so), /usr/lib/x86_64-linux-gnu/libpython3.6m.so)
	GCC_DEFINES3=-DLIBPYTHON_SO='"/usr/lib/x86_64-linux-gnu/libpython3.6m.so"'
endif 

all: pam_python.so pam_python3.so

pam_python.so: pam_python.c
	gcc ${INCLUDE2} ${GCC_FLAGS} ${GCC_DEFINES2} -o pam_python.so pam_python.c ${LIBS2}

pam_python3.so: pam_python3.c
	gcc ${INCLUDE3} ${GCC_FLAGS} ${GCC_DEFINES3} -o pam_python3.so pam_python3.c ${LIBS3}

install:
	cp pam_python3.so /lib/security/pam_python3.so
	cp pam_python.so /lib/security/pam_python.so

clean:
	rm -f pam_python3.so
	rm -f pam_python.so
