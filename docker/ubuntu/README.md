## Ubuntu: A Docker image used to build and test rippled

The code in this repository creates a locked-down Ubuntu image for building and
testing rippled in the GitHub CI pipelines.

Although the images will be built by a CI pipeline in this repository, if
necessary a maintainer can build them manually by following the instructions
below.

### Logging into the Docker registry

To be able to push to GitHub a personal access token is needed, see instructions
[here](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-container-registry#authenticating-with-a-personal-access-token-classic).
In summary, if you do not have a suitable personal access token, generate one
[here](https://github.com/settings/tokens/new?scopes=write:packages).

```shell
DOCKER_REGISTRY=ghcr.io
GITHUB_USER=<your-github-username>
GITHUB_TOKEN=<your-github-personal-access-token>
echo ${GITHUB_TOKEN} | \
docker login ${DOCKER_REGISTRY} -u "${GITHUB_USER}" --password-stdin
```

### Building and pushing the Docker image

The same Dockerfile can be used to build an image for Ubuntu 22.04 and 24.04 by
specifying the `UBUNTU_VERSION` build argument. There are additional arguments
to specify as well, namely `CMAKE_VERSION` and `GCC_VERSION` for the GCC flavor
and `CMAKE_VERSION` and `CLANG_VERSION` for the Clang flavor.

Run the commands below from the current directory containing the Dockerfile.

#### Building the Docker image for GCC.

Ensure you've run the login command above to authenticate with the Docker
registry.

```shell
UBUNTU_VERSION=noble
GCC_VERSION=14
DOCKER_IMAGE=xrplf/ci/${UBUNTU_VERSION}:gcc${GCC_VERSION}

DOCKER_BUILDKIT=1 docker build . \
  --target gcc \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg UBUNTU_VERSION=${UBUNTU_VERSION} \
  --build-arg GCC_VERSION=${GCC_VERSION} \
  --tag ${DOCKER_REGISTRY}/${DOCKER_IMAGE} \
  --platform linux/amd64

docker push ${DOCKER_REGISTRY}/${DOCKER_IMAGE}
```

#### Building the Docker image for Clang.

Ensure you've run the login command above to authenticate with the Docker
registry.

```shell
UBUNTU_VERSION=noble
CLANG_VERSION=18
DOCKER_IMAGE=xrplf/ci/${UBUNTU_VERSION}:clang${CLANG_VERSION}

DOCKER_BUILDKIT=1 docker build . \
  --target clang \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg UBUNTU_VERSION=${UBUNTU_VERSION} \
  --build-arg CLANG_VERSION=${CLANG_VERSION} \
  --tag ${DOCKER_REGISTRY}/${DOCKER_IMAGE} \
  --platform linux/amd64

docker push ${DOCKER_REGISTRY}/${DOCKER_IMAGE}
```
