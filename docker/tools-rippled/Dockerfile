# ====================== BASE IMAGE ======================
ARG DEBIAN_VERSION
FROM debian:${DEBIAN_VERSION} AS base

# Use Bash as the default shell for RUN commands and as the entrypoint.
SHELL ["/bin/bash", "-c"]
ENTRYPOINT ["/bin/bash"]

# Ensure any packages installed directly or indirectly via dpkg do not require
# manual interaction.
ARG DEBIAN_FRONTEND=noninteractive
RUN <<EOF
set -ex
ln -fs /usr/share/zoneinfo/America/Los_Angeles /etc/localtime
apt update
apt upgrade -y
apt install -y tzdata
rm -rf /var/lib/apt/lists/* && apt clean
dpkg-reconfigure --frontend noninteractive tzdata
EOF

# Install tools that are shared by all stages. Run `apt update` again before
# installing the packages to ensure the package lists are up-to-date, which is
# especially important when the image is built from a cached layer.
RUN <<EOF
pkgs=()
pkgs+=(ca-certificates) # Enable TLS verification for HTTPS connections by providing trusted root certificates.
pkgs+=(cmake)           # Required build tool.
pkgs+=(curl)            # Dependency for tools requiring downloading data.
pkgs+=(git)             # Required build tool.
pkgs+=(gpg)             # Dependency for tools requiring signing or encrypting/decrypting.
pkgs+=(jq)              # Pretty printing.
pkgs+=(libc6-dev)       # Required build tool.
pkgs+=(ninja-build)     # Required build tool.
pkgs+=(pipx)            # Package manager for Python applications.
pkgs+=(wget)            # Required build tool.
apt update
apt install -y "${pkgs[@]}"
apt clean
rm -rf /var/lib/apt/lists/*
EOF

# Install Conan.
ARG CONAN_VERSION
ENV PIPX_HOME=/opt/pipx \
    PIPX_BIN_DIR=/usr/bin \
    PIPX_MAN_DIR=/usr/share/man
RUN pipx install --pip-args='--no-cache' conan==${CONAN_VERSION}

# Create the user to switch to, once all packages have been installed.
ARG NONROOT_USER
RUN useradd -ms /bin/bash ${NONROOT_USER}

# ====================== clang-format IMAGE ======================
# Note, we do not install a compiler here.

FROM base AS clang-format

# Install clang-format.
ARG CLANG_FORMAT_VERSION
RUN pipx install --pip-args='--no-cache' clang-format==${CLANG_FORMAT_VERSION}

# Switch to the non-root user.
USER ${NONROOT_USER}
WORKDIR /home/${NONROOT_USER}
