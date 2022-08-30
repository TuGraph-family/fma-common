FROM ubuntu:16.04

RUN apt-get update && apt-get install -y \
    git \
    ssh \
    tar \
    bzip2 \
    gzip \
    ca-certificates \
    g++ \
    cmake

# install boost-1.68
RUN cd ~ && wget https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.bz2 \
    && tar xjf boost_1_68_0.tar.bz2 && cd boost_1_68_0 \
    && ./bootstrap.sh --with-libraries=system,random,thread,filesystem,chrono,atomic,date_time,regex,stacktrace \
    && ./b2 -j16 cxxflags="-std=c++14 -fPIC" install && rm -rf ~/boost_*
