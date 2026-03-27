FROM ubuntu:jammy

RUN apt-get update && apt-get upgrade -y && apt-get install -y libc-bin curl && rm -rf /var/lib/apt/lists/*
RUN curl -OL https://go.dev/dl/go1.24.9.linux-amd64.tar.gz && \
    tar -C /usr/local -xzf go1.24.9.linux-amd64.tar.gz && \
    rm go1.24.9.linux-amd64.tar.gz

ENV GOPATH=/go
ENV PATH=$GOPATH/bin:/usr/local/go/bin:$PATH

RUN mkdir -p "$GOPATH/src" "$GOPATH/bin" && chmod -R 1777 "$GOPATH"

RUN mkdir /client && mkdir /client/test_files && mkdir /var/log/client && mkdir /opt/shared
WORKDIR /client

COPY ./build/my_secure_file_transfer /client/msft

RUN chmod +x /client/msft

WORKDIR /opt/client/
CMD ["go", "test"]
