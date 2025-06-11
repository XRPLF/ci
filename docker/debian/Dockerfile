# ====================== BASE IMAGE ======================
ARG DEBIAN_VERSION=bookworm
FROM debian:${DEBIAN_VERSION} AS base

# Associate the image with the repository.
ARG GITHUB_REPO=XRPLF/ci
LABEL org.opencontainers.image.source=https://github.com/${GITHUB_REPO}

# Ensure any packages installed directly or indirectly via dpkg do not require
# manual interaction.
ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && apt upgrade -y && \
    ln -fs /usr/share/zoneinfo/America/Los_Angeles /etc/localtime && \
    apt install tzdata && \
    dpkg-reconfigure --frontend noninteractive tzdata

# Install tools that are shared by all stages.
RUN apt install -y build-essential \
                        ca-certificates \
                        cmake \
                        curl \
                        git \
                        gpg \
                        libssl-dev \
                        libprotobuf-dev \
                        ninja-build \
                        protobuf-compiler

# Install Conan.
RUN apt install -y pipx
RUN PIPX_HOME=/opt/pipx \
         PIPX_BIN_DIR=/usr/bin \
         PIPX_MAN_DIR=/usr/share/man \
         pipx install conan
RUN apt purge -y pipx

# Create the user to switch to, once all packages have been installed.
ARG NONROOT_USER=ci
RUN useradd -ms /bin/bash ${NONROOT_USER}

# ====================== GCC IMAGE ======================
FROM base AS gcc

# Install GCC.
ARG GCC_VERSION=12
RUN apt install -y gcc-${GCC_VERSION} g++-${GCC_VERSION}
ENV CC=/usr/bin/gcc-${GCC_VERSION}
ENV CXX=/usr/bin/gcc-${GCC_VERSION}

# Clean up unnecessary files to reduce image size.
RUN rm -rf /var/lib/apt/lists/* && apt autoremove -y && apt clean

# Switch to the non-root user.
USER ${NONROOT_USER}
WORKDIR /home/${NONROOT_USER}

# ===================== CLANG IMAGE =====================
FROM base AS clang

# Install Clang. Use the LLVM apt repository to access the latest versions.
ARG CLANG_VERSION=16
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor -o /etc/apt/keyrings/llvm.gpg && \
    printf "%s\n%s\n" \
      "deb [signed-by=/etc/apt/keyrings/llvm.gpg] https://apt.llvm.org/${DEBIAN_VERSION}/ llvm-toolchain-${DEBIAN_VERSION}-${CLANG_VERSION} main" \
      "deb-src [signed-by=/etc/apt/keyrings/llvm.gpg] https://apt.llvm.org/${DEBIAN_VERSION}/ llvm-toolchain-${DEBIAN_VERSION}-${CLANG_VERSION} main" \
      | tee /etc/apt/sources.list.d/llvm.list && \
    apt-get update && \
    apt-get install -t llvm-toolchain-${DEBIAN_VERSION}-${CLANG_VERSION} -y --no-install-recommends clang-${CLANG_VERSION} llvm-${CLANG_VERSION}
ENV CC=/usr/bin/clang-${CLANG_VERSION}
ENV CXX=/usr/bin/clang++-${CLANG_VERSION}

# Clean up unnecessary files to reduce image size.
RUN rm -rf /var/lib/apt/lists/* && apt autoremove -y && apt clean

# Switch to the non-root user.
USER ${NONROOT_USER}
WORKDIR /home/${NONROOT_USER}
