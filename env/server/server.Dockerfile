FROM ubuntu:latest

RUN apt update && apt upgrade -y
RUN apt install libc-bin

RUN mkdir /server
RUN mkdir /server/test_files
RUN mkdir /var/log/server
WORKDIR /server

COPY ./env/test_files/ /server/
COPY ./build/my_secure_file_transfer /server/

RUN chmod +x /server/my_secure_file_transfer
CMD ["/server/my_secure_file_transfer", "server", "-t", "10.5.0.10"]
