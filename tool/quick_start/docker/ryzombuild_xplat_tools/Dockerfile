FROM --platform=linux/amd64 rust:1-bookworm
LABEL org.opencontainers.image.authors="Ryzom Core <https://wiki.ryzom.dev/>"
# Cross-platform tools that are not C++: the quick start does not scan this image for a compiler toolchain
LABEL dev.ryzom.quickstart.toolchain="none"

# The Rust tools of the code repository (tool/servdash_cli), built for a generic Linux (static musl)
# and for Windows (MSVC target through cargo-xwin, which fetches the Windows SDK and CRT on first use)
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  clang lld llvm musl-tools python3 \
  && rm -rf /var/lib/apt/lists/*

RUN rustup target add x86_64-unknown-linux-musl x86_64-pc-windows-msvc \
  && cargo install --locked cargo-xwin \
  && rm -rf /usr/local/cargo/registry

RUN mkdir -p /mnt/nel

WORKDIR /mnt/nel
