FROM ubuntu:16.04

ENV PAM_STACK_NAME=simple_pam

RUN apt-get update && \
    apt-get install -y gcc g++ make libpam-dev emacs wget curl db-util && \
    mkdir -p /opt/include/catch2 && \
    wget https://github.com/catchorg/Catch2/releases/download/v2.10.2/catch.hpp -O /opt/include/catch2/catch.hpp

ADD Makefile /app/Makefile
ADD src /app/src
ADD test /app/test
ADD functional_test/sleep.sh /app/sleep.sh

RUN make clean -C /app/
RUN make -C /app/src && \
    make -C /app test 

ADD functional_test/pam.d/* /etc/pam.d/
ADD functional_test/run_db_test.sh /app/run_db_test.sh
ADD functional_test/init_db.sh /app/init_db.sh

RUN /app/init_db.sh

CMD "/app/server" "--verbose" "--addr"  "0.0.0.0" "--port" "8080" "--stack" $PAM_STACK_NAME 

