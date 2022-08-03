FROM ubuntu:22.04 as runtime
# Runtime packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3-pip perl sox libfst8 libngram2
RUN pip install --no-cache-dir numpy scipy

FROM runtime as build
# Stuff that hopefully won't change
RUN apt-get update && apt-get install -y \
    automake autoconf autoconf-archive libtool \
    bison gcc g++ wget git cmake \
    python3 python3-dev python3-wheel swig \
    libfst-dev libngram-dev
RUN wget https://github.com/cmusphinx/pocketsphinx/archive/03745404eecb7a440308263293caabdb605e7906.tar.gz
RUN tar xf 03745404eecb7a440308263293caabdb605e7906.tar.gz
WORKDIR /pocketsphinx-03745404eecb7a440308263293caabdb605e7906
RUN mkdir build && cd build && cmake .. && make && make install
WORKDIR /
COPY . /sphinxtrain
WORKDIR /sphinxtrain
RUN ./autogen.sh --enable-g2p-decoder && make clean && make install

FROM runtime
RUN ln -s python3 /usr/bin/python
COPY --from=build /usr/local/ /usr/local/
RUN /sbin/ldconfig

# Create a non-root user and use it
RUN useradd -UM sphinxtrain
RUN mkdir /st && chown sphinxtrain:sphinxtrain /st
USER sphinxtrain

# Mount your training directory here
WORKDIR /st

ENTRYPOINT ["sphinxtrain"]
