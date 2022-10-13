FROM ubuntu:22.04 as runtime
# Runtime packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3-pip python-is-python3 perl sox libfst8 libngram2
# Installing the Ubuntu packages gives us too many dependencies :(
RUN pip install --no-cache-dir numpy scipy

FROM runtime as build
# Stuff that hopefully won't change
RUN apt-get update && apt-get install -y \
    gcc g++ git cmake libfst-dev libngram-dev ninja-build
RUN git clone --depth 1 https://github.com/cmusphinx/pocketsphinx.git
WORKDIR /pocketsphinx
RUN cmake -S . -B build -G Ninja && cmake --build build --target install
WORKDIR /
COPY . /sphinxtrain
WORKDIR /sphinxtrain
RUN cmake -S . -B build -G Ninja -DBUILD_G2P=ON -DBUILD_SHARED_LIBS=ON \
    && cmake --build build --target install

FROM runtime
COPY --from=build /usr/local/ /usr/local/
RUN /sbin/ldconfig

# Create a non-root user and use it
RUN useradd -UM sphinxtrain
RUN mkdir /st && chown sphinxtrain:sphinxtrain /st
USER sphinxtrain

# Mount your training directory here
WORKDIR /st

ENTRYPOINT ["sphinxtrain"]
