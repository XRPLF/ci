## GCC: A Docker image with GCC compiler

The code in this repository creates a GCC image based on the
[official GCC repository](https://github.com/docker-library/gcc), backported
to an older Debian release.

The Dockerfiles stored in this directory use a file extension, e.g. `12-bullseye`
to indicate both GCC version and Debian release. This extension also matches the
tag used when pushing the images to [ghcr.io](https://github.com/XRPLF/ci/pkgs/container/gcc).

The Dockerfiles are only altered from [upstream](https://github.com/docker-library/gcc)
in the following ways:

* update the base image to an older Debian release
* remove the "DOCKERFILE IS GENERATED, PLEASE DO NOT EDIT" commment
* indicate the specific `Dockerfile`, including commit, they have been sourced from.

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

Dockerfiles provided in this directory can be used to build an image for different GCC versions
by specifying it as part of the the extension of a Dockerfile, e.g. `Dockerfile.12-bullseye`.

Even though in principle this repository can support different Debian releases,
currently only Dockerfiles for Debian Bullseye are provided.

#### Building the Docker image

In order to build an image, run the commands below from the root directory of
the repository.

Ensure you've run the login command above to authenticate with the Docker
registry.

```shell
GCC_VERSION=12
DEBIAN_VERSION=bullseye
CONTAINER_IMAGE=xrplf/ci/gcc:${GCC_VERSION}-${DEBIAN_VERSION}

docker buildx build . \
  --file docker/gcc/Dockerfile.${GCC_VERSION}-${DEBIAN_VERSION} \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --tag ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```

#### Pushing the Docker image to the GitHub registry

If you want to push the image to the GitHub registry, you can do so with the
following command:

```shell
docker push ${CONTAINER_REGISTRY}/${CONTAINER_IMAGE}
```
