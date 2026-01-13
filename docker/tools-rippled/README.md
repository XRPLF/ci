## tools-rippled: A Docker image used for building rippled

The code in this repository creates a locked-down Ubuntu image for the
verification of rippled source code changes, with appropriate tools installed.

Although the images will be built by a CI pipeline in this repository, if
necessary a maintainer can build them manually by following the instructions
below.

### Building the Docker image

Currently, this Dockerfile can be used to build one the following images:

* `pre-commit` with formatting tools for various languages. This image requires
  parameters:
  * `UBUNTU_VERSION` for selecting the Ubuntu release (recommended `noble`).
  * `PRE_COMMIT_VERSION` for the [pre-commit](https://pre-commit.com/) version.
* `documentation` with tools for building the rippled documentation. This image
  requires parameters:
  * `UBUNTU_VERSION` for selecting the Ubuntu release (recommended `noble`)
  * `CMAKE_VERSION` for the [CMake](https://cmake.org/) version.
  * `DOXYGEN_VERSION` for the [Doxygen](https://www.doxygen.nl/) version.
  * `GCC_VERSION` for the [GCC](https://gcc.gnu.org/) version.
  * `GRAPHVIZ_VERSION` for the [Graphviz](https://graphviz.org/) version.

#### Building the Docker image for pre-commit

In order to build an image, run the commands below from the root directory of
the repository.

```shell
UBUNTU_VERSION=noble
PRE_COMMIT_VERSION=4.2.0
CONTAINER_IMAGE=ghcr.io/xrplf/ci/tools-rippled-pre-commit:latest

docker buildx build . \
  --file docker/tools-rippled/Dockerfile \
  --target pre-commit \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg PRE_COMMIT_VERSION=${PRE_COMMIT_VERSION} \
  --build-arg UBUNTU_VERSION=${UBUNTU_VERSION} \
  --tag ${CONTAINER_IMAGE}
```

#### Building the Docker image for documentation

In order to build an image, run the commands below from the root directory of
the repository.

```shell
UBUNTU_VERSION=noble
CMAKE_VERSION=4.2.1
DOXYGEN_VERSION=1.9.8+ds-2build5
GCC_VERSION=14
GRAPHVIZ_VERSION=2.42.2-9ubuntu0.1
CONTAINER_IMAGE=ghcr.io/xrplf/ci/tools-rippled-documentation:latest

docker buildx build . \
  --file docker/tools-rippled/Dockerfile \
  --target documentation \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg CMAKE_VERSION=${CMAKE_VERSION} \
  --build-arg DOXYGEN_VERSION=${DOXYGEN_VERSION} \
  --build-arg GCC_VERSION=${GCC_VERSION} \
  --build-arg GRAPHVIZ_VERSION=${GRAPHVIZ_VERSION} \
  --build-arg UBUNTU_VERSION=${UBUNTU_VERSION} \
  --tag ${CONTAINER_IMAGE}
```

#### Building the Docker image for signing

In order to build an image, run the commands below from the root directory of
the repository.

```shell
UBUNTU_VERSION=noble
CONTAINER_IMAGE=ghcr.io/xrplf/ci/tools-rippled-signing:latest

docker buildx build . \
  --file docker/tools-rippled/Dockerfile \
  --target signing \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg UBUNTU_VERSION=${UBUNTU_VERSION} \
  --tag ${CONTAINER_IMAGE}
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
docker tag ${CONTAINER_IMAGE} ${CONTAINER_IMAGE}-${GITHUB_USER}
docker push ${CONTAINER_IMAGE}-${GITHUB_USER}
```

This way you can test the image in the `rippled` repository by modifying the
`.github/workflows/pre-commit.yml` and/or `.github/workflows/publish-docs.yml`
files, and then creating a pull request.

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
