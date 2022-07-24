FROM alpine:latest as build
RUN apk add autoconf automake libtool make bison gcc musl-dev git gcc g++ perl cmake
RUN wget -O pocketsphinx-master.zip https://github.com/cmusphinx/pocketsphinx/archive/refs/heads/master.zip
RUN unzip pocketsphinx-master
RUN mkdir pocketsphinx-master/build && cd pocketsphinx-master/build && cmake .. && make && make install
RUN wget -O sphinxtrain-master.zip https://github.com/cmusphinx/sphinxtrain/archive/refs/heads/master.zip
RUN unzip sphinxtrain-master
RUN cd sphinxtrain-master && ./autogen.sh && make install

FROM alpine:latest
RUN apk add perl python3 sox
RUN ln -s python3 /usr/bin/python
COPY --from=build /usr/local/ /usr/local/
