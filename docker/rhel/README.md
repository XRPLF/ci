## RHEL: A Docker image used to build and test rippled

The code in this repository creates a locked-down Red Hat Enterprise Linux
(RHEL) image for building and testing rippled in the GitHub CI pipelines.

Although the images will be built by a CI pipeline in this repository, if
necessary a maintainer can build them manually by following the instructions
below.

### Logging into the Red Hat registry

To be able to read the Universal Base Images from the Red Hat registry, a
personal access  token is needed, which you can get by registering for a
Developer account [here](https://developers.redhat.com) and then creating a
service account [here](https://access.redhat.com/terms-based-registry).

Once created, click on the service account to view its details, and then
navigate to the "Docker Login" tab to get the username and password. A command
is shown that will log you into the Red Hat registry, which looks like this:

```shell
REDHAT_REGISTRY=registry.redhat.io
REDHAT_USER=<your-rhel-username>
REDHAT_TOKEN=<your-rhel-password>
echo ${REDHAT_TOKEN} | \
docker login ${REDHAT_REGISTRY} -u "${REDHAT_USER}" --password-stdin
```

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

The same Dockerfile can be used to build an image for RHEL 9.6 or future
versions by specifying the `RHEL_VERSION` build argument. There are additional
arguments to specify as well, namely `GCC_VERSION` for the GCC flavor and
`CLANG_VERSION` for the Clang flavor.

Run the commands below from the root directory of the repository.

#### Building the Docker image for GCC.

Ensure you've run the login command above to authenticate with the Docker
registry.

```shell
RHEL_VERSION=9.6
GCC_VERSION=13
CONAN_VERSION=2.18.0
GCOVR_VERSION=8.3
CONTAINER_IMAGE=xrplf/ci/rhel-${RHEL_VERSION}:gcc-${GCC_VERSION}

docker buildx build . \
  --file docker/rhel/Dockerfile \
  --target gcc \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg CONAN_VERSION=${CONAN_VERSION} \
  --build-arg GCC_VERSION=${GCC_VERSION} \
  --build-arg GCOVR_VERSION=${GCOVR_VERSION} \
  --build-arg RHEL_VERSION=${RHEL_VERSION} \
  --tag ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```

#### Building the Docker image for Clang.

Ensure you've run the login command above to authenticate with the Docker
registry.

```shell
RHEL_VERSION=9.6
CONAN_VERSION=2.18.0
GCOVR_VERSION=8.3
CONTAINER_IMAGE=xrplf/ci/rhel-${RHEL_VERSION}:clang-${CLANG_VERSION}

docker buildx build . \
  --file docker/rhel/Dockerfile \
  --target clang \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg CONAN_VERSION=${CONAN_VERSION} \
  --build-arg GCOVR_VERSION=${GCOVR_VERSION} \
  --build-arg RHEL_VERSION=${RHEL_VERSION} \
  --tag ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```

#### Running the Docker image

If you want to run the image locally using a cloned `rippled` repository, you
can do so with the following command:

```shell
CODEBASE=<path to the rippled repository>
docker run --user $(id -u):$(id -g) --rm -it -v ${CODEBASE}:/rippled ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```

Note, the above command will assume the identity of the current user in the newly created Docker container.
**This might be exploited by other users with access to the same host (docker instance)**.

The recommended practice is to run Docker in [rootless mode](https://docs.docker.com/engine/security/rootless/),
or use alternative container runtime such as [podman](https://docs.podman.io/en/latest/) which
support [rootless environment](https://github.com/containers/podman/blob/main/docs/tutorials/rootless_tutorial.md).
This will have similar effect as `--user $(id -u):$(id -g)` (making this option redundant and invalid),
while also securing the container from other users on the same host.

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
