# Define the source image from which we will copy GCC. This is needed because
# the `COPY --from XXX` command used in a stage below does not allow `XXX` to
# contain dynamic values supplied via an argument. We are using GCC image rather
# than the official Debian package to access the latest versions, built by the
# GCC team specifically for this Debian release. For build images using official
# distribution packages, see Ubuntu. We set the GCC version to "invalid" to
# satisfy the syntax checker, as it checks if the `FROM` command has a valid
# image, even though it is not used for Clang.
ARG DEBIAN_VERSION
ARG GCC_VERSION=invalid
FROM gcc:${GCC_VERSION}-${DEBIAN_VERSION} AS gcc-src

# ====================== BASE IMAGE ======================
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
dpkg-reconfigure --frontend noninteractive tzdata
EOF

# Install tools that are shared by all stages. Run `apt update` again before
# installing the packages to ensure the package lists are up-to-date, which is
# especially important when the image is built from a cached layer.
RUN <<EOF
pkgs=()
pkgs+=(bison)           # Required build tool.
pkgs+=(ca-certificates) # Enable TLS verification for HTTPS connections by providing trusted root certificates.
pkgs+=(cmake)           # Required build tool.
pkgs+=(curl)            # Dependency for tools requiring downloading data.
pkgs+=(flex)            # Required build tool.
pkgs+=(git)             # Required build tool.
pkgs+=(gpg)             # Dependency for tools requiring signing or encrypting/decrypting.
pkgs+=(jq)              # Pretty printing.
pkgs+=(libc6-dev)       # Required build tool.
pkgs+=(ninja-build)     # Required build tool.
pkgs+=(pipx)            # Package manager for Python applications.
pkgs+=(wget)            # Required build tool.
apt update
apt install -y "${pkgs[@]}"
EOF

# Install Conan.
ARG CONAN_VERSION
RUN PIPX_HOME=/opt/pipx \
    PIPX_BIN_DIR=/usr/bin \
    PIPX_MAN_DIR=/usr/share/man \
    pipx install conan==${CONAN_VERSION}

# Create the user to switch to, once all packages have been installed.
ARG NONROOT_USER
RUN useradd -ms /bin/bash ${NONROOT_USER}

# ====================== GCC IMAGE ======================
FROM base AS gcc

# Copy GCC from the source image, make the package manager aware of its
# existence, and create the necessary symlinks.
COPY --from=gcc-src /usr/local/ /usr/local/
COPY --from=gcc-src /etc/ld.so.conf.d/*.conf /etc/ld.so.conf.d/
RUN <<EOF
set -ex
ldconfig -v
dpkg-divert --divert /usr/bin/gcc.orig --rename /usr/bin/gcc
dpkg-divert --divert /usr/bin/g++.orig --rename /usr/bin/g++
dpkg-divert --divert /usr/bin/gfortran.orig --rename /usr/bin/gfortran
update-alternatives --install /usr/bin/cc cc /usr/local/bin/gcc 999
update-alternatives --install \
  /usr/bin/gcc gcc /usr/local/bin/gcc 100 \
  --slave /usr/bin/g++ g++ /usr/local/bin/g++ \
  --slave /usr/bin/gcc-ar gcc-ar /usr/local/bin/gcc-ar \
  --slave /usr/bin/gcc-nm gcc-nm /usr/local/bin/gcc-nm \
  --slave /usr/bin/gcc-ranlib gcc-ranlib /usr/local/bin/gcc-ranlib \
  --slave /usr/bin/gcov gcov /usr/local/bin/gcov \
  --slave /usr/bin/gcov-tool gcov-tool /usr/local/bin/gcov-tool \
  --slave /usr/bin/gcov-dump gcov-dump /usr/local/bin/gcov-dump \
  --slave /usr/bin/lto-dump lto-dump /usr/local/bin/lto-dump
update-alternatives --auto cc
update-alternatives --auto gcc
EOF
ENV CC=/usr/bin/gcc
ENV CXX=/usr/bin/g++

# Clean up unnecessary files to reduce image size.
RUN rm -rf /var/lib/apt/lists/* && apt clean

# Switch to the non-root user.
USER ${NONROOT_USER}
WORKDIR /home/${NONROOT_USER}

# Create a default Conan profile.
RUN conan profile detect
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

# ===================== CLANG IMAGE =====================
FROM base AS clang

# Install Clang. Use the LLVM apt repository to access the latest versions. We
# must repeat the DEBIAN_VERSION argument here, as it is not inherited from the
# base image.
ARG DEBIAN_VERSION
ARG CLANG_VERSION
RUN <<EOF
set -ex
curl --no-progress-meter https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor -o /etc/apt/keyrings/llvm.gpg
printf "%s\n%s\n" \
  "deb [signed-by=/etc/apt/keyrings/llvm.gpg] https://apt.llvm.org/${DEBIAN_VERSION}/ llvm-toolchain-${DEBIAN_VERSION}-${CLANG_VERSION} main" \
  | tee /etc/apt/sources.list.d/llvm.list
apt update
apt install -t llvm-toolchain-${DEBIAN_VERSION}-${CLANG_VERSION} -y --no-install-recommends clang-${CLANG_VERSION} llvm-${CLANG_VERSION}
EOF
ENV CC=/usr/bin/clang-${CLANG_VERSION}
ENV CXX=/usr/bin/clang++-${CLANG_VERSION}

# Clean up unnecessary files to reduce image size.
RUN rm -rf /var/lib/apt/lists/* && apt clean

# Switch to the non-root user.
USER ${NONROOT_USER}
WORKDIR /home/${NONROOT_USER}

# Create a default Conan profile.
RUN conan profile detect
# Fix the C++ dialect.
RUN sed -i -e 's|^compiler\.cppstd=.*$|compiler.cppstd=20|' ~/.conan2/profiles/default
# Explicitly set the compiler flags. To ensure compatibility with a range of
# Clang compilers, we must also add extra flags that apply to certain versions
# of Clang.
# TODO: Move extra flags into the rippled repository as a custom Conan profile.
RUN <<EOF
cat >>~/.conan2/profiles/default <<EOT
[conf]
tools.build:compiler_executables={"c": "${CC}", "cpp": "${CXX}"}
EOT
if [[ $(clang-${CLANG_VERSION} --version | head -1 | grep -Po 'version \K[0-9]{2}') -ge 20 ]]; then
    cat >>~/.conan2/profiles/default <<EOT
tools.build:cxxflags=['-Wno-missing-template-arg-list-after-template-kw', '-Wno-deprecated-declarations']
EOT
elif [[ $(clang-${CLANG_VERSION} --version | head -1 | grep -Po 'version \K[0-9]{2}') -eq 19 ]]; then
    cat >>~/.conan2/profiles/default <<EOT
tools.build:cxxflags=['-Wno-missing-template-arg-list-after-template-kw']
EOT
fi
EOF
# Print the Conan profile to verify the configuration.
RUN conan profile show
