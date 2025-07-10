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
pkgs+=(jq)              # Pretty printing.
pkgs+=(libc6-dev)       # Required build tool.
pkgs+=(ninja-build)     # Required build tool.
pkgs+=(pipx)            # Package manager for Python applications.
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

# Create the user to switch to, once all packages have been installed.
ARG NONROOT_USER
RUN useradd -ms /bin/bash ${NONROOT_USER}

# ====================== GCC IMAGE ======================
FROM base AS gcc

# This is not inherited from base image, ensure no manual interaction needed.
ARG DEBIAN_FRONTEND=noninteractive

# Install GCC and create the necessary symlinks.
ARG GCC_VERSION
RUN <<EOF
apt-get update
apt-get install -y --no-install-recommends \
    gcc-${GCC_VERSION} \
    g++-${GCC_VERSION}
apt-get clean
rm -rf /var/lib/apt/lists/*
update-alternatives \
    --install /usr/bin/gcc gcc /usr/bin/gcc-${GCC_VERSION} ${GCC_VERSION} \
    --slave /usr/bin/g++ g++ /usr/bin/g++-${GCC_VERSION} \
    --slave /usr/bin/cpp cpp /usr/bin/cpp-${GCC_VERSION} \
    --slave /usr/bin/gcov gcov /usr/bin/gcov-${GCC_VERSION} \
    --slave /usr/bin/gcov-dump gcov-dump /usr/bin/gcov-dump-${GCC_VERSION} \
    --slave /usr/bin/gcov-tool gcov-tool /usr/bin/gcov-tool-${GCC_VERSION}
EOF
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

# Switch to the non-root user.
USER ${NONROOT_USER}
WORKDIR /home/${NONROOT_USER}

# Create a default Conan profile.
RUN <<EOF
conan profile detect
rm -rf /tmp/*
EOF

# Fix the C++ dialect.
RUN sed -i -e 's|^compiler\.cppstd=.*$|compiler.cppstd=20|' ~/.conan2/profiles/default
# Explicitly set the compiler flags.
RUN <<EOF
cat >>~/.conan2/profiles/default <<EOT
[conf]
tools.build:compiler_executables={"c": "${CC}", "cpp": "${CXX}"}
EOT
EOF
# Print the Conan profile to verify the configuration.
RUN conan profile show

# Test the image by compiling a simple C++ program.
RUN --mount=type=bind,source=test,target=/test,readonly <<EOF
cp -r /test test
cd test && ./run.sh && cd ..
rm -rf test
EOF

# ===================== CLANG IMAGE =====================
FROM base AS clang

# This is not inherited from base image, ensure no manual interaction needed.
ARG DEBIAN_FRONTEND=noninteractive

# Install Clang.
ARG CLANG_VERSION
RUN <<EOF
apt-get update
apt-get install -y --no-install-recommends \
    clang-${CLANG_VERSION} \
    llvm-${CLANG_VERSION}
apt-get clean
rm -rf /var/lib/apt/lists/*
EOF
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

# Switch to the non-root user.
USER ${NONROOT_USER}
WORKDIR /home/${NONROOT_USER}

# Create a default Conan profile.
RUN <<EOF
conan profile detect
rm -rf /tmp/*
EOF

# Fix the C++ dialect.
RUN sed -i -e 's|^compiler\.cppstd=.*$|compiler.cppstd=20|' ~/.conan2/profiles/default
# Explicitly set the compiler flags. To ensure compatibility with a range of
# Clang compilers, we must also add extra flags that apply to certain versions
# of Clang.
# TODO: Move extra flags into the rippled repository as a custom Conan profile.
RUN <<EOF
CXX_VER=$(${CXX} -dumpversion)
CXX_VER=${CXX_VER%%.*}
cat >>~/.conan2/profiles/default <<EOT
[conf]
tools.build:compiler_executables={"c": "${CC}", "cpp": "${CXX}"}
EOT
if [[ ${CXX_VER} -ge 19 ]]; then
    cat >>~/.conan2/profiles/default <<EOT
tools.build:cxxflags=['-Wno-missing-template-arg-list-after-template-kw']
EOT
fi
EOF
# Print the Conan profile to verify the configuration.
RUN conan profile show

# Test the image by compiling a simple C++ program.
RUN --mount=type=bind,source=test,target=/test,readonly <<EOF
cp -r /test test
cd test && ./run.sh && cd ..
rm -rf test
EOF
