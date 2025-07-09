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

Run the commands below from the root directory of the repository.

#### Building the Docker image for GCC.

Ensure you've run the login command above to authenticate with the Docker
registry.

```shell
NONROOT_USER=${USER}
DEBIAN_VERSION=bookworm
GCC_VERSION=12
CONAN_VERSION=2.18.0
GCOVR_VERSION=8.3
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
  --build-arg NONROOT_USER=${NONROOT_USER} \
  --tag ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```

#### Building the Docker image for Clang.

Ensure you've run the login command above to authenticate with the Docker
registry.

```shell
NONROOT_USER=${USER}
DEBIAN_VERSION=bookworm
CLANG_VERSION=17
CONAN_VERSION=2.18.0
GCOVR_VERSION=8.3
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
  --build-arg NONROOT_USER=${NONROOT_USER} \
  --tag ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```

#### Running the Docker image

If you want to run the image locally using a cloned `rippled` repository, you
can do so with the following command:

```shell
CODEBASE=<path to the rippled repository>
docker run --rm -it -v ${CODEBASE}:/rippled ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```

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
```

#### Pushing the Docker image to the GitHub registry

If you want to push the image to the GitHub registry, you can do so with the
following command:

```shell
docker push ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```
