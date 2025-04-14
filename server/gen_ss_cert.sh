#!/bin/sh

openssl req -nodes -new -x509 -subj="/C=/ST=/L=/O=/CN=idunno_server" -keyout server.key -out server.cert

