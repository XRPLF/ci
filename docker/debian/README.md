## Debian: A Docker image used to build and test rippled

The code in this repository creates a locked-down Debian image for building and
testing rippled in the GitHub CI pipelines.

Although the images will be built by a CI pipeline in this repository, if
necessary a maintainer can build them manually by following the instructions
below.

### Logging into the GitHub registry

To be able to push a Docker image to the GitHub registry, a personal access
token is needed, see instructions [here](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-container-registry#authenticating-with-a-personal-access-token-classic).
In summary, if you do not have a suitable personal access token, generate one
[here](https://github.com/settings/tokens/new?scopes=write:packages).

```shell
CONTAINER_REGISTRY=ghcr.io
GITHUB_USER=<your-github-username>
GITHUB_TOKEN=<your-github-personal-access-token>
echo ${GITHUB_TOKEN} | \
docker login ${CONTAINER_REGISTRY} -u "${GITHUB_USER}" --password-stdin
```

### Building and pushing the Docker image

The same Dockerfile can be used to build an image for Debian Bookworm or future
versions by specifying the `DEBIAN_VERSION` build argument. There are additional
arguments to specify as well, namely `GCC_VERSION` for the GCC flavor and
`CLANG_VERSION` for the Clang flavor.

Build image for `gcc` supports packaging.

In order to build an image, run the commands below from the root directory of
the repository.

#### Note on old Debian releases

This image supports variety of releases of Debian, GCC and Clang.

The GCC binaries are sourced from [Docker "Official Image" for gcc](https://github.com/docker-library/gcc)
with an important caveat - in order to install a GCC release in older
Debian versions, we keep a local copy of `Dockerfile` from the above repository,
backported to an older Debian base image. Such dockerfiles are stored in this
directory with special file extension, e.g. `gcc-12-bullseye`. They are not altered from
the source, except for change of the base image to older Debian version. They also
show in a comment the specific `Dockerfile` they have been sourced from.

If you want to build a Docker image for GCC and an older Debian version, you should
first build GCC using an appropriate image, giving it the _exact_ name and tag as
passed later to the main `Dockerfile` as `BASE_IMAGE`. This may require significant
CPU resources and take some time (e.g. 30 minutes using 40 cores) and it's needed
to ensure that we do not use an old GCC release with known, and fixed, bugs.

For example:

```shell
docker buildx build . --progress plain --file docker/debian/Dockerfile.gcc-12-bullseye --tag localhost.localdomain/gcc:12-bullseye
```

#### Building the Docker image for GCC

Ensure you've run the login command above to authenticate with the Docker
registry.

```shell
DEBIAN_VERSION=bookworm
GCC_VERSION=12
CONAN_VERSION=2.19.1
GCOVR_VERSION=8.3
CMAKE_VERSION=3.31.6
CONTAINER_IMAGE=xrplf/ci/debian-${DEBIAN_VERSION}:gcc-${GCC_VERSION}

docker buildx build . \
  --file docker/debian/Dockerfile \
  --target gcc \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg CONAN_VERSION=${CONAN_VERSION} \
  --build-arg DEBIAN_VERSION=${DEBIAN_VERSION} \
  --build-arg GCC_VERSION=${GCC_VERSION} \
  --build-arg GCOVR_VERSION=${GCOVR_VERSION} \
  --build-arg CMAKE_VERSION=${CMAKE_VERSION} \
  --tag ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```

If you have prepared a GCC image for an older Debian version, you also need
to explicitly set `BASE_IMAGE` build argument, e.g.

```shell
DEBIAN_VERSION=bullseye
GCC_VERSION=12
CONAN_VERSION=2.19.1
GCOVR_VERSION=8.3
CMAKE_VERSION=3.31.6
BASE_IMAGE=localhost.localdomain/gcc:12-bullseye
CONTAINER_IMAGE=xrplf/ci/debian-${DEBIAN_VERSION}:gcc-${GCC_VERSION}

docker buildx build . \
  --file docker/debian/Dockerfile \
  --target gcc \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg CONAN_VERSION=${CONAN_VERSION} \
  --build-arg DEBIAN_VERSION=${DEBIAN_VERSION} \
  --build-arg GCC_VERSION=${GCC_VERSION} \
  --build-arg GCOVR_VERSION=${GCOVR_VERSION} \
  --build-arg CMAKE_VERSION=${CMAKE_VERSION} \
  --build-arg BASE_IMAGE=${BASE_IMAGE} \
  --tag ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```

#### Building the Docker image for Clang

Ensure you've run the login command above to authenticate with the Docker
registry.

```shell
DEBIAN_VERSION=bookworm
CLANG_VERSION=17
CONAN_VERSION=2.19.1
GCOVR_VERSION=8.3
CMAKE_VERSION=3.31.6
CONTAINER_IMAGE=xrplf/ci/debian-${DEBIAN_VERSION}:clang-${CLANG_VERSION}

docker buildx build . \
  --file docker/debian/Dockerfile \
  --target clang \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg CLANG_VERSION=${CLANG_VERSION} \
  --build-arg CONAN_VERSION=${CONAN_VERSION} \
  --build-arg DEBIAN_VERSION=${DEBIAN_VERSION} \
  --build-arg GCOVR_VERSION=${GCOVR_VERSION} \
  --build-arg CMAKE_VERSION=${CMAKE_VERSION} \
  --tag ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```

#### Running the Docker image

If you want to run the image locally using a cloned `rippled` repository, you
can do so with the following command:

```shell
CODEBASE=<path to the rippled repository>
docker run --user $(id -u):$(id -g) --rm -it -v ${CODEBASE}:/rippled ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
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

Once inside the container you can run the following commands to build `rippled`:

```shell
BUILD_TYPE=Debug
cd /rippled
# Remove any existing data from previous builds on the host machine.
rm -rf CMakeCache.txt CMakeFiles build || true
# Install dependencies via Conan.
conan install . --build missing --settings build_type=${BUILD_TYPE} \
  -o xrpld=True -o tests=True -o unity=True
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

#### Pushing the Docker image to the GitHub registry

If you want to push the image to the GitHub registry, you can do so with the
following command:

```shell
docker push ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```
