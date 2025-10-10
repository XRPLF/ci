## Ubuntu: A Docker image used to build and test rippled

The code in this repository creates a locked-down Ubuntu image for building and
testing rippled in the GitHub CI pipelines.

Although the images will be built by a CI pipeline in this repository, if
necessary a maintainer can build them manually by following the instructions
below.

### Building the Docker image

The same Dockerfile can be used to build an image for Ubuntu 22.04 and 24.04 or
future versions by specifying the `UBUNTU_VERSION` build argument. There are
additional arguments to specify as well, namely `GCC_VERSION` for the GCC flavor
and `CLANG_VERSION` for the Clang flavor.

Only the build image for `gcc` supports packaging.

#### Building the Docker image for GCC

In order to build the image for GCC, run the commands below from the root
directory of the repository.

```shell
UBUNTU_VERSION=noble
GCC_VERSION=14
CONAN_VERSION=2.19.1
GCOVR_VERSION=8.3
CMAKE_VERSION=4.1.0
MOLD_VERSION=2.40.4
CONTAINER_IMAGE=ghcr.io/xrplf/ci/ubuntu-${UBUNTU_VERSION}:gcc-${GCC_VERSION}

docker buildx build . \
  --file docker/ubuntu/Dockerfile \
  --target gcc \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg CONAN_VERSION=${CONAN_VERSION} \
  --build-arg GCC_VERSION=${GCC_VERSION} \
  --build-arg GCOVR_VERSION=${GCOVR_VERSION} \
  --build-arg CMAKE_VERSION=${CMAKE_VERSION} \
  --build-arg MOLD_VERSION=${MOLD_VERSION} \
  --build-arg UBUNTU_VERSION=${UBUNTU_VERSION} \
  --tag ${CONTAINER_IMAGE}
```

#### Building the Docker image for Clang

In order to build the image for Clang, run the commands below from the root
directory of the repository.

```shell
UBUNTU_VERSION=noble
CLANG_VERSION=18
CONAN_VERSION=2.19.1
GCOVR_VERSION=8.3
CMAKE_VERSION=4.1.0
MOLD_VERSION=2.40.4
CONTAINER_IMAGE=ghcr.io/xrplf/ci/ubuntu-${UBUNTU_VERSION}:clang-${CLANG_VERSION}

docker buildx build . \
  --file docker/ubuntu/Dockerfile \
  --target clang \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg CLANG_VERSION=${CLANG_VERSION} \
  --build-arg CONAN_VERSION=${CONAN_VERSION} \
  --build-arg GCOVR_VERSION=${GCOVR_VERSION} \
  --build-arg CMAKE_VERSION=${CMAKE_VERSION} \
  --build-arg MOLD_VERSION=${MOLD_VERSION} \
  --build-arg UBUNTU_VERSION=${UBUNTU_VERSION} \
  --tag ${CONTAINER_IMAGE}
```

### Running the Docker image

If you want to run the image locally using a cloned `rippled` repository, you
can do so with the following command:

```shell
CODEBASE=<path to the rippled repository>
docker run --user $(id -u):$(id -g) --rm -it -v ${CODEBASE}:/rippled ${CONTAINER_IMAGE}
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
