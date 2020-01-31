FROM ubuntu:16.04


RUN apt-get update && \
    apt-get install -y gcc g++ make libpam-dev emacs wget && \
    mkdir -p /opt/include/catch2 && \
    wget https://github.com/catchorg/Catch2/releases/download/v2.10.2/catch.hpp -O /opt/include/catch2/catch.hpp

ADD . /app/
RUN make clean -C /app
RUN make -C /app/src && \
    make -C /app/src test 

CMD ["/app/functional_test/sleep.sh"]

