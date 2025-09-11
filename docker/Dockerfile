# Multi-stage build for KIO programming language
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libllvm-dev \
    llvm-dev \
    clang \
    libnlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /kio

# Copy source code
COPY . .

# Build KIO
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release \
             -DKIO_BUILD_LSP=ON \
             -DKIO_ENABLE_JIT=ON \
             -DKIO_ENABLE_PARALLEL=ON && \
    make -j$(nproc)

# Runtime stage
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Copy binaries
COPY --from=builder /kio/build/kio /usr/local/bin/
COPY --from=builder /kio/build/kio-lsp /usr/local/bin/

# Copy examples
COPY --from=builder /kio/examples /opt/kio/examples

WORKDIR /workspace

ENTRYPOINT ["kio"]
