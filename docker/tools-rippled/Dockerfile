# ====================== BASE IMAGE ======================
ARG UBUNTU_VERSION
FROM ubuntu:${UBUNTU_VERSION} AS base

# Use Bash as the default shell for RUN commands, using the options
# `set -o errexit -o pipefail`, and as the entrypoint.
SHELL ["/bin/bash", "-e", "-o", "pipefail", "-c"]
ENTRYPOINT ["/bin/bash"]

# Ensure any packages installed directly or indirectly via dpkg do not require
# manual interaction.
ARG DEBIAN_FRONTEND=noninteractive
RUN <<EOF
ln -fs /usr/share/zoneinfo/America/Los_Angeles /etc/localtime
apt-get update
apt-get upgrade -y
apt-get install -y --no-install-recommends tzdata
apt-get clean
rm -rf /var/lib/apt/lists/*
dpkg-reconfigure --frontend noninteractive tzdata
EOF

# Install tools that are shared by all stages. Run `apt-get update` again before
# installing the packages to ensure the package lists are up-to-date, which is
# especially important when the image is built from a cached layer.
RUN <<EOF
pkgs=()
pkgs+=(ca-certificates) # Enable TLS verification for HTTPS connections by providing trusted root certificates.
pkgs+=(curl)            # Dependency for tools requiring downloading data.
pkgs+=(git)             # Required build tool.
pkgs+=(gpg)             # Dependency for tools requiring signing or encrypting/decrypting.
pkgs+=(gpg-agent)       # Dependency for tools requiring signing or encrypting/decrypting.
pkgs+=(jq)              # Pretty printing.
pkgs+=(pipx)            # Package manager for Python applications.
pkgs+=(wget)            # Required build tool.
pkgs+=(vim)             # Text editor.
apt-get update
apt-get install -y --no-install-recommends "${pkgs[@]}"
apt-get clean
rm -rf /var/lib/apt/lists/*
EOF

ENV PIPX_HOME=/opt/pipx \
    PIPX_BIN_DIR=/usr/bin \
    PIPX_MAN_DIR=/usr/share/man

# ====================== CLANG-FORMAT IMAGE ======================
# Note, we do not install a compiler here.

FROM base AS clang-format

# This is not inherited from base image.
ARG DEBIAN_FRONTEND=noninteractive

# Install clang-format.
ARG CLANG_FORMAT_VERSION
ARG PRE_COMMIT_VERSION
RUN pipx install --pip-args='--no-cache' clang-format==${CLANG_FORMAT_VERSION} && \
    pipx install --pip-args='--no-cache' pre-commit==${PRE_COMMIT_VERSION}

ENV HOME=/root
WORKDIR ${HOME}

# ====================== DOCUMENTATION IMAGE ======================
# Note, we do not install a compiler here.

FROM base AS documentation

# This is not inherited from base image.
ARG DEBIAN_FRONTEND=noninteractive

# Install doxygen and graphviz.
ARG DOXYGEN_VERSION
ARG GRAPHVIZ_VERSION
RUN <<EOF
apt-get update
apt-get install -y doxygen=${DOXYGEN_VERSION} graphviz=${GRAPHVIZ_VERSION}
apt-get clean
rm -rf /var/lib/apt/lists/*
EOF

ENV HOME=/root
WORKDIR ${HOME}

# ====================== PRETTIER IMAGE ======================
# Note, we do not install a compiler here.

FROM base AS prettier

# This is not inherited from base image.
ARG DEBIAN_FRONTEND=noninteractive

# Install node and npm via nvm, and then install prettier, see also
# https://nodejs.org/en/download.
ARG NVM_VERSION
ARG NODE_VERSION
ARG PRETTIER_VERSION
RUN <<EOF
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v${NVM_VERSION}/install.sh | bash
\. "${HOME}/.nvm/nvm.sh"
nvm install ${NODE_VERSION}
export NPM_CONFIG_REGISTRY=https://registry.npmjs.org
npm install -g --no-audit --no-fund npm@latest
npm install -g --no-audit --no-fund prettier@${PRETTIER_VERSION}
EOF

ENV HOME=/root
WORKDIR ${HOME}
