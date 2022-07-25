FROM alpine:latest as build
# Stuff that hopefully won't change
RUN apk add autoconf automake libtool make bison gcc musl-dev git gcc g++ perl cmake python3 python3-dev py3-numpy-dev py3-wheel py3-pip swig
RUN git clone https://github.com/dhdaines/sequitur-g2p.git
# Don't use pip or build as they will rebuild numpy from scratch :(
RUN cd sequitur-g2p && python3 setup.py bdist_wheel

# Now build things that are susceptible to change
RUN wget -O pocketsphinx-master.zip https://github.com/cmusphinx/pocketsphinx/archive/refs/heads/master.zip
RUN unzip pocketsphinx-master
RUN mkdir pocketsphinx-master/build && cd pocketsphinx-master/build && cmake .. && make && make install
RUN wget -O sphinxtrain-master.zip https://github.com/cmusphinx/sphinxtrain/archive/refs/heads/master.zip
RUN unzip sphinxtrain-master
RUN cd sphinxtrain-master && ./autogen.sh && make install

FROM alpine:latest
RUN apk add perl python3 py3-pip py3-scipy sox
RUN ln -s python3 /usr/bin/python
COPY --from=build /usr/local/ /usr/local/
COPY --from=build /sequitur-g2p/dist/*.whl /
RUN pip3 install *.whl && rm *.whl
