# ====================== BASE IMAGE ======================
ARG RHEL_VERSION
FROM registry.redhat.io/ubi${RHEL_VERSION%.*}/s2i-base:${RHEL_VERSION} AS base

# Use Bash as the default shell for RUN commands, using the options
# `set -o errexit -o pipefail`, and as the entrypoint.
SHELL ["/bin/bash", "-e", "-o", "pipefail", "-c"]
ENTRYPOINT ["/bin/bash"]

# Update the packages installed on the system.
RUN dnf update-minimal -y --security --sec-severity=Important --sec-severity=Critical

# Install tools that are shared by all stages.
RUN <<EOF
pkgs=()
pkgs+=(ca-certificates)  # Enable TLS verification for HTTPS connections by providing trusted root certificates.
pkgs+=(cmake)            # Required build tool.
pkgs+=(file)             # Required packaging tool.
pkgs+=(git)              # Required build tool.
pkgs+=(gpg)              # Dependency for tools requiring signing or encrypting/decrypting.
pkgs+=(gnupg2)           # Dependency for tools requiring signing or encrypting/decrypting.
pkgs+=(jq)               # JSON manipulation.
pkgs+=(libstdc++-static) # Required to statically link libraries into rippled.
pkgs+=(ninja-build)      # Required build tool.
pkgs+=(perl-FindBin)     # Required to compile OpenSSL.
pkgs+=(python3-jinja2)   # Required build tool.
pkgs+=(python3-pip)      # Package manager for Python applications.
pkgs+=(rpm-build)        # Required packaging tool.
pkgs+=(rpmdevtools)      # Required packaging tool.
pkgs+=(vim)              # Text editor.
pkgs+=(wget)             # Required build tool.
dnf install -y --allowerasing --setopt=tsflags=nodocs "${pkgs[@]}"
dnf clean -y all
rm -rf /var/cache/dnf/*
EOF

# Install Python-based tools.
ARG CONAN_VERSION
ARG GCOVR_VERSION
RUN pip install --no-cache conan==${CONAN_VERSION} gcovr==${GCOVR_VERSION}

# ====================== GCC IMAGE ======================
FROM base AS gcc

# These are not inherited from the base image.
ARG GCC_VERSION
ARG RHEL_VERSION

# Install GCC. Red Hat installs GCC as a Software Collection (SCL) package,
# where typically you would run `scl enable gcc-toolset-X` to open another Bash
# shell with the GCC toolset enabled. To avoid having to do so, we delete the
# default GCC packages first, and then reference the GCC toolset binaries
# directly.
RUN <<EOF
dnf remove -y gcc gcc-c++
dnf install -y --setopt=tsflags=nodocs gcc-toolset-${GCC_VERSION}-gcc gcc-toolset-${GCC_VERSION}-gcc-c++
dnf clean -y all
rm -rf /var/cache/dnf/*
update-alternatives \
  --install /usr/bin/cc cc /opt/rh/gcc-toolset-${GCC_VERSION}/root/usr/bin/gcc 999
update-alternatives \
  --install /usr/bin/c++ c++ /opt/rh/gcc-toolset-${GCC_VERSION}/root/usr/bin/g++ 999
update-alternatives \
  --install /usr/bin/gcc gcc /opt/rh/gcc-toolset-${GCC_VERSION}/root/usr/bin/gcc 100 \
  --slave /usr/bin/g++ g++ /opt/rh/gcc-toolset-${GCC_VERSION}/root/usr/bin/g++ \
  --slave /usr/bin/cpp cpp /opt/rh/gcc-toolset-${GCC_VERSION}/root/usr/bin/cpp \
  --slave /usr/bin/gcov gcov /opt/rh/gcc-toolset-${GCC_VERSION}/root/usr/bin/gcov \
  --slave /usr/bin/gcov-dump gcov-dump /opt/rh/gcc-toolset-${GCC_VERSION}/root/usr/bin/gcov-dump-${GCC_VERSION} \
  --slave /usr/bin/gcov-tool gcov-tool /opt/rh/gcc-toolset-${GCC_VERSION}/root/usr/bin/gcov-tool-${GCC_VERSION}
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

# Explicitly set the compiler flags, as well as a custom flag that specifies the
# distribution name and version. This is important to avoid conflicts between
# builds using the same compiler but different distribution versions.
RUN <<EOF
cat >> $(conan config home)/global.conf <<EOT
tools.build:compiler_executables={"c": "${CC}", "cpp": "${CXX}"}
user.package:distro="rhel-${RHEL_VERSION}"
tools.info.package_id:confs=["user.package:distro"]
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
ARG RHEL_VERSION

# Install Clang. Note that in the RHEL UBIs, we cannot choose the Clang version
# to install and we get what is available. Some build dependencies require GCC
# to be also available.
RUN <<EOF
dnf install -y --setopt=tsflags=nodocs clang llvm gcc gcc-c++
dnf clean -y all
rm -rf /var/cache/dnf/*
update-alternatives \
  --install /usr/bin/cc cc /usr/bin/clang 999
update-alternatives \
  --install /usr/bin/c++ c++ /usr/bin/clang++ 999
update-alternatives --auto cc
update-alternatives --auto c++
EOF
# Set the compiler environment variables to point to the GCC binaries.
ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++

# Check that the installed Clang version is not older than the minimum required.
ARG MINIMUM_CLANG_VERSION=16
RUN <<EOF
CC_VER=$(${CC} -dumpversion)
CC_VER=${CC_VER%%.*}
if [[ ${CC_VER} -lt ${MINIMUM_CLANG_VERSION} ]]; then
  echo "ERROR: 'clang -dumpversion' gives '${CC_VER}', which is less than minimum required '${MINIMUM_CLANG_VERSION}'."
  exit 1
fi
CXX_VER=$(${CXX} -dumpversion)
CXX_VER=${CXX_VER%%.*}
if [[ ${CXX_VER} -lt ${MINIMUM_CLANG_VERSION} ]]; then
  echo "ERROR: clang++ -dumpversion gives '${CXX_VER}', which is less than minimum required '${MINIMUM_CLANG_VERSION}'."
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

# Explicitly set the compiler flags, as well as a custom flag that specifies the
# distribution name and version. This is important to avoid conflicts between
# builds using the same compiler but different distribution versions.
RUN <<EOF
cat >> $(conan config home)/global.conf <<EOT
tools.build:compiler_executables={"c": "${CC}", "cpp": "${CXX}"}
user.package:distro="rhel-${RHEL_VERSION}"
tools.info.package_id:confs=["user.package:distro"]
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
