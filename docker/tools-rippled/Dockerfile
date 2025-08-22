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
pkgs+=(python3-venv)    # Python environment management tool.
pkgs+=(python3-pip)     # Package manager for Python applications.
pkgs+=(wget)            # Required build tool.
pkgs+=(vim)             # Text editor.
apt-get update
apt-get install -y --no-install-recommends "${pkgs[@]}"
apt-get clean
rm -rf /var/lib/apt/lists/*
EOF

# Prepare the environment for Python-based tools.
ENV VIRTUAL_ENV=/opt/venv
RUN python3 -m venv ${VIRTUAL_ENV}
ENV PATH=${VIRTUAL_ENV}/bin:${PATH}

# ====================== PRE-COMMIT IMAGE ======================
# Note, we do not install a compiler here.

FROM base AS pre-commit

# This is not inherited from base image.
ARG DEBIAN_FRONTEND=noninteractive

# Install clang-format, pre-commit and pre-commit-hooks.
ARG CLANG_FORMAT_VERSION
ARG PRE_COMMIT_VERSION
ARG PRE_COMMIT_HOOKS_VERSION
RUN pip install --no-cache \
    clang-format==${CLANG_FORMAT_VERSION} \
    pre-commit==${PRE_COMMIT_VERSION} \
    pre-commit-hooks==${PRE_COMMIT_HOOKS_VERSION}

# Install Node.JS, NPM, and prettier.
ARG NODE_VERSION
ARG NPM_VERSION
ARG PRETTIER_VERSION
ENV NPM_CONFIG_REGISTRY=https://registry.npmjs.org
RUN <<EOF
curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | \
  gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg
echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_${NODE_VERSION%%.*}.x nodistro main" | \
 tee /etc/apt/sources.list.d/nodesource.list
apt-get update
apt-get install -y --no-install-recommends nodejs=${NODE_VERSION}-1nodesource1
apt-get clean
rm -rf /var/lib/apt/lists/*
npm install -g --no-audit --no-fund npm@${NPM_VERSION}
npm install -g --no-audit --no-fund prettier@${PRETTIER_VERSION}
npm cache clean --force
EOF

ENV HOME=/root
WORKDIR ${HOME}

# ====================== DOCUMENTATION IMAGE ======================

FROM base AS documentation

# This is not inherited from base image.
ARG DEBIAN_FRONTEND=noninteractive

# Install cmake.
ARG CMAKE_VERSION
RUN pip install --no-cache cmake==${CMAKE_VERSION}

# Install make, GCC and create the necessary symlinks.
ARG GCC_VERSION
RUN <<EOF
apt-get update
apt-get install -y --no-install-recommends \
  make \
  gcc-${GCC_VERSION} \
  g++-${GCC_VERSION}
apt-get clean
rm -rf /var/lib/apt/lists/*
update-alternatives \
  --install /usr/bin/cc cc /usr/bin/gcc-${GCC_VERSION} 999
update-alternatives \
  --install /usr/bin/c++ c++ /usr/bin/g++-${GCC_VERSION} 999
update-alternatives \
  --install /usr/bin/gcc gcc /usr/bin/gcc-${GCC_VERSION} 100 \
  --slave /usr/bin/g++ g++ /usr/bin/g++-${GCC_VERSION} \
  --slave /usr/bin/gcov gcov /usr/bin/gcov-${GCC_VERSION} \
  --slave /usr/bin/gcov-dump gcov-dump /usr/bin/gcov-dump-${GCC_VERSION} \
  --slave /usr/bin/gcov-tool gcov-tool /usr/bin/gcov-tool-${GCC_VERSION}
update-alternatives --auto cc
update-alternatives --auto c++
update-alternatives --auto gcc
EOF
# Set the compiler environment variables to point to the GCC binaries.
ENV CC=/usr/bin/gcc
ENV CXX=/usr/bin/g++

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
