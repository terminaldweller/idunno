FROM golang:1.23-alpine3.21 AS builder
WORKDIR /server
COPY go.sum go.mod /server/
RUN go mod download
COPY *.go /server/
RUN go build

FROM alpine:3.21 AS certbuilder
RUN apk add openssl
WORKDIR /certs
RUN openssl req -nodes -new -x509 -subj="/C=/ST=/L=/O=/CN=idunno_server" -keyout server.key -out server.cert

FROM alpine:3.21
ENV HOME=/home/user
RUN set -eux; \
  adduser -u 1001 -D -h "$HOME" user; \
  chown -R user:user "$HOME"
COPY --from=certbuilder /certs /certs
COPY --from=builder /server/idunno_server "$HOME/idunno_server"
RUN chown user:user "$HOME/idunno_server"
USER user
ENTRYPOINT ["/home/user/idunno_server"]
