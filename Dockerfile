FROM ubuntu
ENV TZ=Europe/Moscow
RUN apt-get clean && apt-get update -y 
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install libboost-all-dev tzdata git sudo build-essential autoconf libtool libssl-dev zlib1g-dev librdkafka-dev pkg-config cmake gcc git g++ mysql-client libmysqlclient-dev libssl-dev && apt-get clean
RUN git clone --depth 1 --branch poco-1.12.4-release https://github.com/pocoproject/poco.git && cd poco && mkdir cmake-build && cd cmake-build && cmake .. && make && sudo make install && cd .. && cd ..
RUN git clone https://github.com/tdv/redis-cpp.git && mkdir redis-cpp/build && cd redis-cpp/build && cmake .. && make && sudo make install && cd ../..
RUN git clone https://github.com/edenhill/librdkafka.git && cd librdkafka && ./configure && make && sudo make install && cd ..
RUN git clone https://github.com/mfontanini/cppkafka && cd cppkafka && mkdir build && cd build && cmake .. && make && sudo make install && cd .. && cd ..
ADD http://date.jsontest.com /etc/builddate
RUN git clone https://github.com/vindosVP/po_architecture.git
RUN sudo ldconfig
RUN cd po_architecture && mkdir build && cd build && cmake .. && make
ENTRYPOINT [ "po_architecture/build/app" ]