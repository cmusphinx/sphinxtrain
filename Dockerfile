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
    libfst-dev libngram-dev ninja-build
RUN git clone --depth 1 https://github.com/cmusphinx/pocketsphinx.git
WORKDIR /pocketsphinx
RUN cmake -S . -B build -G Ninja && cmake --build build --target install
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
