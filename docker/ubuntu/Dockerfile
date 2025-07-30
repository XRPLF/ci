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
pkgs+=(cmake)           # Required build tool.
pkgs+=(curl)            # Dependency for tools requiring downloading data.
pkgs+=(dpkg-dev)        # Required packaging tool.
pkgs+=(file)            # Required packaging tool.
pkgs+=(git)             # Required build tool.
pkgs+=(gpg)             # Dependency for tools requiring signing or encrypting/decrypting.
pkgs+=(gpg-agent)       # Dependency for tools requiring signing or encrypting/decrypting.
pkgs+=(jq)              # JSON manipulation.
pkgs+=(libc6-dev)       # Required build tool.
pkgs+=(ninja-build)     # Required build tool.
pkgs+=(pipx)            # Package manager for Python applications.
pkgs+=(python3-jinja2)  # Required build tool.
pkgs+=(vim)             # Text editor.
pkgs+=(wget)            # Required build tool.
apt-get update
apt-get install -y --no-install-recommends "${pkgs[@]}"
apt-get clean
rm -rf /var/lib/apt/lists/*
EOF

# Install Python-based tools.
ARG CONAN_VERSION
ARG GCOVR_VERSION
ENV PIPX_HOME=/opt/pipx \
    PIPX_BIN_DIR=/usr/bin \
    PIPX_MAN_DIR=/usr/share/man
RUN pipx install --pip-args='--no-cache' conan==${CONAN_VERSION} && \
    pipx install --pip-args='--no-cache' gcovr==${GCOVR_VERSION}

# ====================== GCC IMAGE ======================
FROM base AS gcc

# This is not inherited from the base image.
ARG DEBIAN_FRONTEND=noninteractive

# Install GCC and create the necessary symlinks. We only support packaging with
# gcc because of the hard package dependencies from libtool to gcc
ARG GCC_VERSION
RUN <<EOF
apt-get update
apt-get install -y --no-install-recommends \
  gcc-${GCC_VERSION} \
  g++-${GCC_VERSION} \
  debhelper
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

# Check that the installed GCC version matches the expected version.
RUN <<EOF
CC_VER=$(${CC} -dumpversion)
CC_VER=${CC_VER%%.*}
if [[ "${CC_VER}" != "${GCC_VERSION}" ]]; then
  echo "ERROR: 'gcc -dumpversion' gives '${CC_VER}', which does not match expected version '${GCC_VERSION}'."
  exit 1
fi
CXX_VER=$(${CXX} -dumpversion)
CXX_VER=${CXX_VER%%.*}
if [[ "${CXX_VER}" != "${GCC_VERSION}" ]]; then
  echo "ERROR: g++ -dumpversion gives '${CXX_VER}', which does not match expected version '${GCC_VERSION}'."
  exit 1
fi
EOF

# Set the Conan home directory, so the users of this image can find the default
# profile.
ENV HOME=/root
WORKDIR ${HOME}
ENV CONAN_HOME=${HOME}/.conan2

# Create a default Conan profile.
RUN <<EOF
conan profile detect
rm -rf /tmp/*
EOF

# Explicitly set the compiler flags.
RUN <<EOF
cat >> $(conan config home)/global.conf <<EOT
tools.build:compiler_executables={"c": "${CC}", "cpp": "${CXX}"}
EOT
EOF

# Print the Conan profile to verify the configuration.
RUN conan profile show && conan config show 'core*'

# Test the image by compiling a simple C++ program.
RUN --mount=type=bind,source=test,target=/test,readonly <<EOF
cp -r /test test
cd test && ./run.sh gcc
cd ..
rm -rf test
EOF

# ===================== CLANG IMAGE =====================
FROM base AS clang

# This is not inherited from the base image.
ARG DEBIAN_FRONTEND=noninteractive

# Install Clang. Some build dependencies require GCC to be also available.
ARG CLANG_VERSION
RUN <<EOF
apt-get update
apt-get install -y --no-install-recommends \
  clang-${CLANG_VERSION} \
  llvm-${CLANG_VERSION} \
  gcc \
  g++
apt-get clean
rm -rf /var/lib/apt/lists/*
update-alternatives \
  --install /usr/bin/cc cc /usr/bin/clang-${CLANG_VERSION} 999
update-alternatives \
  --install /usr/bin/c++ c++ /usr/bin/clang++-${CLANG_VERSION} 999
update-alternatives \
  --install /usr/bin/clang clang /usr/bin/clang-${CLANG_VERSION} 100 \
  --slave /usr/bin/clang++ clang++ /usr/bin/clang++-${CLANG_VERSION}
update-alternatives \
  --install /usr/bin/llvm-cov llvm-cov /usr/bin/llvm-cov-${CLANG_VERSION} 100
update-alternatives --auto cc
update-alternatives --auto c++
update-alternatives --auto clang
update-alternatives --auto llvm-cov
EOF
# Set the compiler environment variables to point to the GCC binaries.
ENV CC=/usr/bin/clang-${CLANG_VERSION}
ENV CXX=/usr/bin/clang++-${CLANG_VERSION}

# Check that the installed Clang version matches the expected version.
RUN <<EOF
CC_VER=$(${CC} -dumpversion)
CC_VER=${CC_VER%%.*}
if [[ "${CC_VER}" != "${CLANG_VERSION}" ]]; then
  echo "ERROR: 'clang -dumpversion' gives '${CC_VER}', which does not match expected version '${CLANG_VERSION}'."
  exit 1
fi
CXX_VER=$(${CXX} -dumpversion)
CXX_VER=${CXX_VER%%.*}
if [[ "${CXX_VER}" != "${CLANG_VERSION}" ]]; then
  echo "ERROR: clang++ -dumpversion gives '${CXX_VER}', which does not match expected version '${CLANG_VERSION}'."
  exit 1
fi
EOF

# Set the Conan home directory, so the users of this image can find the default
# profile.
ENV HOME=/root
WORKDIR ${HOME}
ENV CONAN_HOME=${HOME}/.conan2

# Create a default Conan profile.
RUN <<EOF
conan profile detect
rm -rf /tmp/*
EOF

# Explicitly set the compiler flags.
RUN <<EOF
cat >> $(conan config home)/global.conf <<EOT
tools.build:compiler_executables={"c": "${CC}", "cpp": "${CXX}"}
EOT
EOF

# Print the Conan profile to verify the configuration.
RUN conan profile show && conan config show 'core*'

# Test the image by compiling a simple C++ program.
RUN --mount=type=bind,source=test,target=/test,readonly <<EOF
cp -r /test test
cd test && ./run.sh clang
cd ..
rm -rf test
EOF
