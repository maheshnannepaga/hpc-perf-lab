FROM ubuntu:24.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential make && rm -rf /var/lib/apt/lists/*
WORKDIR /workspace
COPY . .
RUN make
CMD ["./bin/gemm_bench", "--size", "512", "--block-size", "32", "--repetitions", "3", "--verify"]
