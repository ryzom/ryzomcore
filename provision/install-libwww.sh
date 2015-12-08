#!/bin/sh

set -e

NAME=w3c-libwww-5.4.0
FILE="${NAME}.tgz"
URL="http://www.w3.org/Library/Distribution/${FILE}"

wget $URL
tar -xzvf ${FILE}
(cd "$NAME" && ./configure --prefix=$HOME/protobuf && make && make install)
