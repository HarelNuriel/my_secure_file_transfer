#!/bin/bash

echo "Checking if $(pwd)/log exists."
if [ ! -d "./log" ]; then
  echo "Directory does not Exists."
  echo "Creating $(pwd)/log."
  mkdir log
else
  echo "Directory Exists."
fi

echo "Building the project."
cmake --build .
echo "Done."

echo "Setting up the server's docker."
docker stop my_secure_file_transfer-server-1
docker rm my_secure_file_transfer-server-1
docker image rm msft
docker build . --tag=msft
docker-compose -f ./compose.yaml up -d
echo "Done."

echo "Running the client."
./my_secure_file_transfer client -t 10.5.0.10