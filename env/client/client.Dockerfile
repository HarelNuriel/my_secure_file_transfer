FROM ubuntu:jammy

RUN apt-get update && apt-get upgrade -y && rm -rf /var/lib/apt/lists/*
RUN apt-get install -y libc-bin=2.42-13

RUN mkdir /client && mkdir /client/test_files && mkdir /var/log/client
WORKDIR /client

COPY .. /client/test_files/
COPY ./build/my_secure_file_transfer /client/msft

RUN chmod +x /client/msft
CMD ["/client/msft", "client", "-t", "10.5.0.10", "-l", "/var/log/client/client.log"]
