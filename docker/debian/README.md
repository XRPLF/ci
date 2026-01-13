## Debian: A Docker image used to build and test rippled

The code in this repository creates a locked-down Debian image for building and
testing rippled in the GitHub CI pipelines.

Although the images will be built by a CI pipeline in this repository, if
necessary a maintainer can build them manually by following the instructions
below.

### Building the Docker image

The same Dockerfile can be used to build an image for different Debian releases
by specifying the `DEBIAN_VERSION` build argument. There are additional
arguments to specify as well, namely `GCC_VERSION` for the GCC flavor and
`CLANG_VERSION` for the Clang flavor.

Only the build image for `gcc` supports packaging.

#### Note on old GCC binaries

This image supports variety of releases of Debian, GCC and Clang. In order to
support current GCC versions on an older Debian releases, we rely on `gcc`
images backported from the [official GCC repository](https://github.com/docker-library/gcc).

Hence, depending on the Debian release used, the GCC binaries are sourced from
either of:

- for `DEBIAN_VERSION=bookworm`: `gcc:${GCC_VERSION}-${DEBIAN_VERSION}`, produced in
  the [official GCC repository](https://github.com/docker-library/gcc)
- for `DEBIAN_VERSION=bullseye`: `ghcr.io/xrplf/ci/gcc:${GCC_VERSION}-${DEBIAN_VERSION}`,
  produced in [this repository](https://github.com/XRPLF/ci/pkgs/container/gcc)

#### Building the Docker image for GCC

In order to build the image for GCC, run the commands below from the root
directory of the repository.

```shell
DEBIAN_VERSION=bookworm
GCC_VERSION=12
CCACHE_VERSION=4.12.2
CMAKE_VERSION=4.2.1
CONAN_VERSION=2.24.0
GCOVR_VERSION=8.3
MOLD_VERSION=2.40.4
RUST_VERSION=1.91.1
CONTAINER_IMAGE=ghcr.io/xrplf/ci/debian-${DEBIAN_VERSION}:gcc-${GCC_VERSION}

docker buildx build . \
  --file docker/debian/Dockerfile \
  --target gcc \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg DEBIAN_VERSION=${DEBIAN_VERSION} \
  --build-arg GCC_VERSION=${GCC_VERSION} \
  --build-arg CCACHE_VERSION=${CCACHE_VERSION} \
  --build-arg CMAKE_VERSION=${CMAKE_VERSION} \
  --build-arg CONAN_VERSION=${CONAN_VERSION} \
  --build-arg GCOVR_VERSION=${GCOVR_VERSION} \
  --build-arg MOLD_VERSION=${MOLD_VERSION} \
  --build-arg RUST_VERSION=${RUST_VERSION} \
  --tag ${CONTAINER_IMAGE}
```

In order to build a GCC image for Bullseye, you also need to explicitly set
`BASE_IMAGE` build argument, e.g.

```shell
# The version of the distro to use.
DEBIAN_VERSION=bullseye
# The version of GCC to use, including corresponding base image.
GCC_VERSION=12
BASE_IMAGE=ghcr.io/xrplf/ci/gcc:${GCC_VERSION}-bullseye
# The versions of the tools to use.
CCACHE_VERSION=4.12.2
CMAKE_VERSION=4.2.1
CONAN_VERSION=2.24.0
GCOVR_VERSION=8.3
MOLD_VERSION=2.40.4
RUST_VERSION=1.91.1
CONTAINER_IMAGE=ghcr.io/xrplf/ci/debian-${DEBIAN_VERSION}:gcc-${GCC_VERSION}

docker buildx build . \
  --file docker/debian/Dockerfile \
  --target gcc \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg DEBIAN_VERSION=${DEBIAN_VERSION} \
  --build-arg GCC_VERSION=${GCC_VERSION} \
  --build-arg BASE_IMAGE=${BASE_IMAGE} \
  --build-arg CCACHE_VERSION=${CCACHE_VERSION} \
  --build-arg CMAKE_VERSION=${CMAKE_VERSION} \
  --build-arg CONAN_VERSION=${CONAN_VERSION} \
  --build-arg GCOVR_VERSION=${GCOVR_VERSION} \
  --build-arg MOLD_VERSION=${MOLD_VERSION} \
  --build-arg RUST_VERSION=${RUST_VERSION} \
  --tag ${CONTAINER_IMAGE}
```

#### Building the Docker image for Clang

In order to build an image for Clang, run the commands below from the root
directory of the repository.

```shell
# The version of the distro to use.
DEBIAN_VERSION=bookworm
# The version of Clang to use.
CLANG_VERSION=17
# The versions of the tools to use.
CCACHE_VERSION=4.12.2
CMAKE_VERSION=4.2.1
CONAN_VERSION=2.24.0
GCOVR_VERSION=8.3
MOLD_VERSION=2.40.4
RUST_VERSION=1.91.1
CONTAINER_IMAGE=ghcr.io/xrplf/ci/debian-${DEBIAN_VERSION}:clang-${CLANG_VERSION}

docker buildx build . \
  --file docker/debian/Dockerfile \
  --target clang \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg DEBIAN_VERSION=${DEBIAN_VERSION} \
  --build-arg CLANG_VERSION=${CLANG_VERSION} \
  --build-arg CCACHE_VERSION=${CCACHE_VERSION} \
  --build-arg CMAKE_VERSION=${CMAKE_VERSION} \
  --build-arg CONAN_VERSION=${CONAN_VERSION} \
  --build-arg GCOVR_VERSION=${GCOVR_VERSION} \
  --build-arg MOLD_VERSION=${MOLD_VERSION} \
  --build-arg RUST_VERSION=${RUST_VERSION} \
  --tag ${CONTAINER_IMAGE}
```

### Running the Docker image

If you want to run the image locally using a cloned `rippled` repository, you
can do so with the following command:

```shell
CODEBASE=<path to the rippled repository>
docker run --user $(id -u):$(id -g) --rm -it \
  --mount type=bind,source=${CODEBASE},target=/rippled \
  ${CONTAINER_IMAGE}
```

Note, the above command will assume the identity of the current user in the
newly created Docker container.
**This might be exploited by other users with access to the same host (docker
instance)**.

The recommended practice is to run Docker in [rootless mode](https://docs.docker.com/engine/security/rootless/),
or use alternative container runtime such as [podman](https://docs.podman.io/en/latest/) which
support [rootless environment](https://github.com/containers/podman/blob/main/docs/tutorials/rootless_tutorial.md).
This will have similar effect as `--user $(id -u):$(id -g)` (making this option
redundant and invalid), while also securing the container from other users on
the same host.

If you see an error such as `bash: /root/.bashrc: Permission denied` and the
prompt shows `I have no name!`, then exit the container and run it again without
the `--user $(id -u):$(id -g)` option, or run it in rootless mode.

#### Caching Conan dependencies

You can further customize the `docker run` command by adding a volume mount for
holding the Conan cache (the "p" directory), e.g.:

```shell
docker run --user $(id -u):$(id -g) --rm -it \
  --mount type=bind,source=${CODEBASE},target=/rippled \
  --mount type=volume,source=conan,target=/root/.conan2/p \
  ${CONTAINER_IMAGE}
```

This avoids the need to build the dependencies each time you run the image.

#### Building the binary

Once inside the container you can run the following commands to build `rippled`:

```shell
BUILD_TYPE=Debug
cd /rippled
# Remove any existing data from previous builds on the host machine.
rm -rf CMakeCache.txt CMakeFiles build || true
# Install dependencies via Conan.
conan remote add --force --index 0 xrplf https://conan.ripplex.io
conan install . --build missing --settings:all build_type=${BUILD_TYPE} \
  --options:host '&:tests=True' --options:host '&:xrpld=True'
# Configure the build with CMake.
cd build
cmake -DCMAKE_TOOLCHAIN_FILE:FILEPATH=build/generators/conan_toolchain.cmake \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
# Build and test rippled. Setting the parallelism too high, e.g. to $(nproc),
# can result in an error like "gmake[2]: ...... Killed".
PARALLELISM=2
cmake --build . -j ${PARALLELISM}
./rippled --unittest --unittest-jobs ${PARALLELISM}
```

### Pushing the Docker image

#### Logging into the GitHub registry

To be able to push a Docker image to the GitHub registry, a personal access
token is needed, see instructions [here](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-container-registry#authenticating-with-a-personal-access-token-classic).
In summary, if you do not have a suitable personal access token, generate one
[here](https://github.com/settings/tokens/new?scopes=write:packages).

```shell
GITHUB_USER=<your-github-username>
GITHUB_TOKEN=<your-github-personal-access-token>
echo ${GITHUB_TOKEN} | docker login ghcr.io -u "${GITHUB_USER}" --password-stdin
```

#### Pushing to the GitHub registry

To push the image to the GitHub registry, you can do so with the following
command, whereby we append your username to not overwrite existing images:

```shell
docker tag ${CONTAINER_IMAGE} ${CONTAINER_IMAGE}-sha-${GITHUB_USER}
docker push ${CONTAINER_IMAGE}-sha-${GITHUB_USER}
```

This way you can test the image in the `rippled` repository by modifying the
`image_sha` entry in `.github/scripts/strategy-matrix/linux.json` for the
relevant configuration, and then creating a pull request.

Note, if you or the CI pipeline are pushing an image for the first time, it will
be private by default. You will need to go to the
[packages page](https://github.com/orgs/XRPLF/packages), select the relevant
package, then "Package settings", and after clicking the "Change visibility"
button make it "Public". In addition, on that same page, under "Manage Actions
access" click the "Add repository" button, select the `ci` repository, and grant
it "Admin" access.

#### Note on macOS

If you are using macOS and wish to push an image to the GitHub registry for use
in GitHub Actions, you will need to append `--platform linux/amd64` to the
`docker buildx build` commands above.
