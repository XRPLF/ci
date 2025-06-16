# ====================== BASE IMAGE ======================
ARG UBUNTU_VERSION=jammy
FROM ubuntu:${UBUNTU_VERSION} AS base

# Use Bash as the default shell for RUN commands.
SHELL ["/bin/bash", "-c"]

# Associate the image with the repository.
ARG GITHUB_REPO=XRPLF/ci
LABEL org.opencontainers.image.source=https://github.com/${GITHUB_REPO}

# Ensure any packages installed directly or indirectly via dpkg do not require
# manual interaction.
ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && apt upgrade -y
RUN set -ex ;\
    ln -fs /usr/share/zoneinfo/America/Los_Angeles /etc/localtime ;\
    apt install -y tzdata ;\
    dpkg-reconfigure --frontend noninteractive tzdata

# Install tools that are shared by all stages.
RUN <<EOF
    pkgs=()
    pkgs+=(build-essential) # Required build tools.
    pkgs+=(ca-certificates) # Enable TLS verification for HTTPS connections by providing trusted root certificates.
    pkgs+=(cmake)           # Required build tool.
    pkgs+=(curl)            # Dependency for tools requiring downloading data.
    pkgs+=(git)             # Required build tool.
    pkgs+=(gpg)             # Dependency for tools requiring signing or encrypting/decrypting.
    pkgs+=(jq)              # Pretty printing.
    pkgs+=(ninja-build)     # Required build tool.
    pkgs+=(pipx)            # Package manager for Python applications.
    apt install -y "${pkgs[@]}"
EOF

# Install Conan.
ARG CONAN_VERSION=2.17.0
RUN PIPX_HOME=/opt/pipx \
    PIPX_BIN_DIR=/usr/bin \
    PIPX_MAN_DIR=/usr/share/man \
    pipx install conan==${CONAN_VERSION}

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
RUN rm -rf /var/lib/apt/lists/* && apt clean

# Switch to the non-root user.
USER ${NONROOT_USER}
WORKDIR /home/${NONROOT_USER}

# Create a default Conan profile.
RUN conan profile detect

# ===================== CLANG IMAGE =====================
FROM base AS clang

# Install Clang.
ARG CLANG_VERSION=16
RUN apt install -y clang-${CLANG_VERSION} llvm-${CLANG_VERSION}
ENV CC=/usr/bin/clang-${CLANG_VERSION}
ENV CXX=/usr/bin/clang++-${CLANG_VERSION}

# Clean up unnecessary files to reduce image size.
RUN rm -rf /var/lib/apt/lists/* && apt clean

# Switch to the non-root user.
USER ${NONROOT_USER}
WORKDIR /home/${NONROOT_USER}

# Create a default Conan profile.
RUN conan profile detect
