FROM ubuntu:22.04

# When running `valgrind` in the container, make sure to limit the number of
# file descriptors. Set `ulimit -n 1024`.
RUN apt update -y && \
    apt install -y --no-install-recommends \
        build-essential gdb valgrind bear \
        ruby && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /work/
