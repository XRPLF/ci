# We want to use the subscription-free Red Hat UBI images, but those only
# provide a subset of the packages available in the full Red Hat Enterprise
# Linux (RHEL) distribution. To get some of the packages we need, we use those
# provided by the Rocky Linux project, which is a community-developed operating
# system designed to be a 100% bug-for-bug compatible alternative to RHEL.
ARG RHEL_VERSION
FROM rockylinux/rockylinux:${RHEL_VERSION%.*} AS rocky
RUN dnf -y install bison flex

# ====================== BASE IMAGE ======================
FROM registry.redhat.io/ubi${RHEL_VERSION%.*}/s2i-base:${RHEL_VERSION} AS base

# Use Bash as the default shell for RUN commands and as the entrypoint.
SHELL ["/bin/bash", "-c"]
ENTRYPOINT ["/bin/bash"]

# Update the packages installed on the system.
RUN dnf -y update-minimal --security --sec-severity=Important --sec-severity=Critical

# Install tools that are shared by all stages.
RUN <<EOF
pkgs=()
pkgs+=(ca-certificates)  # Enable TLS verification for HTTPS connections by providing trusted root certificates.
pkgs+=(cmake)            # Required build tool.
pkgs+=(file)             # Required packaging tool.
pkgs+=(git)              # Required build tool.
pkgs+=(gpg)              # Dependency for tools requiring signing or encrypting/decrypting.
pkgs+=(jq)               # Pretty printing.
pkgs+=(libstdc++-static) # Required to statically link libraries into rippled.
pkgs+=(ninja-build)      # Required build tool.
pkgs+=(perl-FindBin)     # Required to compile OpenSSL.
pkgs+=(python3-pip)      # Package manager for Python applications.
pkgs+=(rpm-build)        # Required packaging tool.
pkgs+=(rpmdevtools)      # Required packaging tool.
pkgs+=(wget)             # Required build tool.
dnf install -y --setopt=tsflags=nodocs "${pkgs[@]}"
dnf -y clean all
rm -rf /var/cache/dnf/*
EOF

# Copy Bison and Flex from the Rocky Linux image.
COPY --from=rocky /usr/bin/bison /usr/bin/bison
COPY --from=rocky /usr/bin/flex /usr/bin/flex

# Install Python-based tools.
ARG CONAN_VERSION
ARG GCOVR_VERSION
RUN pip install --no-cache conan==${CONAN_VERSION} gcovr==${GCOVR_VERSION}

# Create the user to switch to, once all packages have been installed.
ARG NONROOT_USER
RUN useradd -ms /bin/bash ${NONROOT_USER}

# ====================== GCC IMAGE ======================
FROM base AS gcc

# Install GCC.
ARG GCC_VERSION
RUN <<EOF
dnf install -y --setopt=tsflags=nodocs gcc-toolset-${GCC_VERSION}-gcc gcc-toolset-${GCC_VERSION}-gcc-c++
dnf -y clean all
rm -rf /var/cache/dnf/*
EOF
ENV CC=/usr/bin/gcc
ENV CXX=/usr/bin/g++

# Switch to the non-root user.
USER ${NONROOT_USER}
WORKDIR /home/${NONROOT_USER}

# Fix the Conan user home directory as it otherwise will point to the
# /opt/app-root/src/.conan2 directory.
ENV CONAN_HOME=/home/${NONROOT_USER}/.conan2

# Create a default Conan profile.
RUN conan profile detect
# Fix the C++ dialect.
RUN sed -i -e 's|^compiler\.cppstd=.*$|compiler.cppstd=20|' ${CONAN_HOME}/profiles/default
# Explicitly set the compiler flags.
RUN <<EOF
cat >>${CONAN_HOME}/profiles/default <<EOT
[conf]
tools.build:compiler_executables={"c": "${CC}", "cpp": "${CXX}"}
EOT
EOF
# Print the Conan profile to verify the configuration.
RUN conan profile show

# ===================== CLANG IMAGE =====================
FROM base AS clang

# Install Clang. Note that in the RHEL UBIs, we cannot choose the Clang version
# to install and we get what is available.
RUN <<EOF
dnf install -y --setopt=tsflags=nodocs clang llvm
dnf -y clean all
rm -rf /var/cache/dnf/*
EOF
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
# Fix the C++ dialect.
RUN sed -i -e 's|^compiler\.cppstd=.*$|compiler.cppstd=20|' ${CONAN_HOME}/profiles/default
# Explicitly set the compiler flags. To ensure compatibility with a range of
# Clang compilers, we must also add extra flags that apply to certain versions
# of Clang.
# TODO: Move extra flags into the rippled repository as a custom Conan profile.
RUN <<EOF
cat >>${CONAN_HOME}/profiles/default <<EOT
[conf]
tools.build:compiler_executables={"c": "${CC}", "cpp": "${CXX}"}
EOT
if [[ $(clang --version | head -1 | grep -Po 'version \K[0-9]{2}') -ge 20 ]]; then
    cat >>${CONAN_HOME}/profiles/default <<EOT
tools.build:cxxflags=['-Wno-missing-template-arg-list-after-template-kw', '-Wno-deprecated-declarations']
EOT
elif [[ $(clang --version | head -1 | grep -Po 'version \K[0-9]{2}') -eq 19 ]]; then
    cat >>${CONAN_HOME}/profiles/default <<EOT
tools.build:cxxflags=['-Wno-missing-template-arg-list-after-template-kw']
EOT
fi
EOF
# Print the Conan profile to verify the configuration.
RUN conan profile show
