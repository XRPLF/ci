## tools-rippled: A Docker image used for building rippled

The code in this repository creates a locked-down Ubuntu image for the verification
of rippled source code changes, with appropriate tools installed.

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

Currently this Dockerfile can be used to build one image:

* `clang-format` with C++ formatting tools. This image requires parameters:
  * `UBUNTU_VERSION` for selection of Ubuntu release (recommended `noble`)
  * `CLANG_FORMAT_VERSION` for [clang-format](http://clang.llvm.org/docs/ClangFormat.html) version
  * `PRE_COMMIT_VERSION` for [pre-commit](https://pre-commit.com/) version

Run the commands below from the current directory containing the Dockerfile to build an image.

#### Building the Docker image for clang-format

Ensure you've run the login command above to authenticate with the Docker
registry.

```shell
NONROOT_USER=${USER}
UBUNTU_VERSION=noble
CLANG_FORMAT_VERSION=18.1.8
PRE_COMMIT_VERSION=4.2.0
CONTAINER_IMAGE=xrplf/ci/tools-rippled-clang-format:latest

docker buildx build . \
  --target clang-format \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg CLANG_FORMAT_VERSION=${CLANG_FORMAT_VERSION} \
  --build-arg PRE_COMMIT_VERSION=${PRE_COMMIT_VERSION} \
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
