FROM alpine:latest as runtime
# Runtime packages
RUN apk add --no-cache perl python3 py3-pip py3-numpy sox

FROM runtime as build
# Stuff that hopefully won't change
RUN apk add --no-cache autoconf automake libtool make bison gcc musl-dev git gcc g++ perl cmake python3 python3-dev py3-numpy-dev py3-wheel py3-pip swig
RUN git clone https://github.com/dhdaines/sequitur-g2p.git
# Don't use pip or build as they will rebuild numpy from scratch :(
RUN cd sequitur-g2p && python3 setup.py bdist_wheel

# Now build things that are susceptible to change
RUN wget https://github.com/cmusphinx/pocketsphinx/archive/03745404eecb7a440308263293caabdb605e7906.zip
RUN unzip 03745404eecb7a440308263293caabdb605e7906
WORKDIR /pocketsphinx-03745404eecb7a440308263293caabdb605e7906
RUN mkdir build && cd build && cmake .. && make && make install
WORKDIR /
COPY . /sphinxtrain
WORKDIR /sphinxtrain
RUN ./autogen.sh && make clean && make install

FROM runtime
RUN ln -s python3 /usr/bin/python
COPY --from=build /usr/local/ /usr/local/
COPY --from=build /sequitur-g2p/dist/*.whl /
RUN pip3 install *.whl && rm *.whl

# Why in the world Docker runs everything as root by default, I will
# never know.  Did they even stop to think that this was a bad idea?
RUN adduser -u 1000 -DHD sphinxtrain
RUN mkdir /st && chown sphinxtrain:sphinxtrain /st
USER sphinxtrain

# Mount your training directory here
WORKDIR /st

ENTRYPOINT ["sphinxtrain"]
