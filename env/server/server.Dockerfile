FROM ubuntu:latest

RUN apt update && apt upgrade -y
RUN apt install libc-bin

RUN mkdir /server && mkdir /server/test_files && mkdir /var/log/server && mkdir /server/data
WORKDIR /server

COPY ./build/my_secure_file_transfer /server/

RUN chmod +x /server/my_secure_file_transfer
CMD ["/server/my_secure_file_transfer", "server", "-t", "10.5.0.10", "-s", "/server/data"]
