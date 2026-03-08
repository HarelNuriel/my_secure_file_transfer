#!/bin/bash

cmake --build .
cp my_secure_file_transfer test/

docker stop my_secure_file_transfer-server-1
docker rm my_secure_file_transfer-server-1
docker build ./test/
docker-compose -f ./test/compose.yaml up -d

./my_secure_file_transfer client -t 10.5.0.10