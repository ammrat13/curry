FROM ubuntu:22.04

RUN apt update -y && \
    apt install -y --no-install-recommends build-essential valgrind ruby bear && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /work/
