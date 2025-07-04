## tools-rippled: A Docker image used for building rippled

The code in this repository creates a locked-down Ubuntu image for building and
testing rippled in the GitHub CI pipelines, with additional tools installed.

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

The same Dockerfile can be used to build a set of images:

* `coverage` for reporting unit tests coverage. This image requires parameters
  * `GCOVR_VERSION` for [gcovr](https://gcovr.com/en/stable/) version
  * `GCC_VERSION` for the GCC version
  * `CONAN_VERSION` for [Conan](https://docs.conan.io/2/) version
* `clang-format` for C++ format tool. This image requires parameters
  * `CLANG_FORMAT_VERSION` for [clang-format](http://clang.llvm.org/docs/ClangFormat.html) version
  * `CONAN_VERSION` for [Conan](https://docs.conan.io/2/) version

To build either image, run the commands below from the current directory containing the Dockerfile.

#### Building the Docker image for clang-format

Ensure you've run the login command above to authenticate with the Docker
registry.

```shell
NONROOT_USER=${USER}
UBUNTU_VERSION=noble
CONAN_VERSION=2.18.0
CLANG_FORMAT_VERSION=18.1.8
CONTAINER_IMAGE=xrplf/ci/tools-rippled-clang-format:latest

docker buildx build . \
  --target clang-format \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg CONAN_VERSION=${CONAN_VERSION} \
  --build-arg CLANG_FORMAT_VERSION=${CLANG_FORMAT_VERSION} \
  --build-arg NONROOT_USER=${NONROOT_USER} \
  --build-arg UBUNTU_VERSION=${UBUNTU_VERSION} \
  --tag ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```

#### Building the Docker image for coverage.

Ensure you've run the login command above to authenticate with the Docker
registry.

```shell
NONROOT_USER=${USER}
UBUNTU_VERSION=noble
CONAN_VERSION=2.18.0
GCC_VERSION=14
GCOVR_VERSION=8.3
CONTAINER_IMAGE=xrplf/ci/tools-rippled-coverage:latest

docker buildx build . \
  --target coverage \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg CONAN_VERSION=${CONAN_VERSION} \
  --build-arg GCC_VERSION=${GCC_VERSION} \
  --build-arg GCOVR_VERSION=${GCOVR_VERSION} \
  --build-arg NONROOT_USER=${NONROOT_USER} \
  --build-arg UBUNTU_VERSION=${UBUNTU_VERSION} \
  --tag ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```

#### Pushing the Docker image to the GitHub registry

If you want to push the image to the GitHub registry, you can do so with the
following command:

```shell
docker push ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```
