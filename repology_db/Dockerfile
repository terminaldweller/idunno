FROM python:3.13-alpine3.21 AS pybuilder
RUN apk update && \
      apk upgrade && \
      apk add --no-cache git yajl-dev libpq-dev=17.4-r0 build-base cmake
RUN git clone https://github.com/repology/libversion && \
      cd libversion && \
      mkdir build && \
      cd build && \
      cmake .. && \
      make && \
      make install
WORKDIR /
RUN git clone https://github.com/repology/repology-updater && \
      cd repology-updater && \
      pip install -r requirements.txt
