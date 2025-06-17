# ====================== GCC IMAGE ======================
ARG RHEL_VERSION=9.6
FROM registry.redhat.io/ubi${RHEL_VERSION%.*}/s2i-base:${RHEL_VERSION} AS base

# Change to the root user to install packages.
USER root

# Use Bash as the default shell for RUN commands and as the entrypoint.
SHELL ["/bin/bash", "-c"]
ENTRYPOINT ["/bin/bash"]

# Associate the image with the repository.
ARG GITHUB_REPO=XRPLF/ci
LABEL org.opencontainers.image.source=https://github.com/${GITHUB_REPO}

# Install tools that are shared by all stages.
RUN <<EOF
    pkgs=()
    pkgs+=(ca-certificates)  # Enable TLS verification for HTTPS connections by providing trusted root certificates.
    pkgs+=(cmake)            # Required build tool.
    pkgs+=(jq)               # Pretty printing.
    pkgs+=(libstdc++-static) # Required to statically link libraries into rippled.
    pkgs+=(ninja-build)      # Required build tool.
    pkgs+=(perl-FindBin)     # Required to compile OpenSSL.
    pkgs+=(python3-pip)      # Package manager for Python applications.
    dnf install -y --setopt=tsflags=nodocs "${pkgs[@]}"
EOF

# Install Conan.
ARG CONAN_VERSION=2.17.0
RUN pip install conan==${CONAN_VERSION}

# Create the user to switch to, once all packages have been installed.
ARG NONROOT_USER=ci
RUN useradd -ms /bin/bash ${NONROOT_USER}

# ====================== GCC IMAGE ======================
FROM base AS gcc

# Install GCC.
ARG GCC_VERSION=14
RUN dnf install -y --setopt=tsflags=nodocs gcc-toolset-${GCC_VERSION}-gcc gcc-toolset-${GCC_VERSION}-gcc-c++

# Switch to the non-root user.
USER ${NONROOT_USER}
WORKDIR /home/${NONROOT_USER}

# Fix the Conan user home directory as it otherwise will point to the
# /opt/app-root/src/.conan2 directory.
ENV CONAN_HOME=/home/${NONROOT_USER}/.conan2

# Create a default Conan profile.
RUN conan profile detect

# ===================== CLANG IMAGE =====================
FROM base AS clang

# Install Clang. Note that in the RHEL UBIs, we cannot choose the Clang version
# to install and we get what is available.
RUN dnf install -y --setopt=tsflags=nodocs clang llvm
ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++

# Switch to the non-root user.
USER ${NONROOT_USER}
WORKDIR /home/${NONROOT_USER}

# Fix the Conan user home directory as it otherwise will point to the
# /opt/app-root/src/.conan2 directory.
ENV CONAN_HOME=/home/${NONROOT_USER}/.conan2

# Create a default Conan profile.
RUN conan profile detect
# To ensure compatibility with a range of Clang compilers, we must add an extra
# flag that applies to Clang 19 and later versions.
RUN <<EOF
if [[ $(clang --version | head -1 | grep -Po 'version \K[0-9]{2}') -ge 19 ]]; then
    cat >>${CONAN_HOME}/profiles/default <<EOT
[conf]
tools.build:cxxflags=['-Wno-missing-template-arg-list-after-template-kw']
EOT
fi
EOF

USER root
