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

### Building the Docker image

Dockerfiles provided in this directory can be used to build an image for
different GCC versions by specifying it as part of the extension of a
Dockerfile, e.g. `Dockerfile.12-bullseye`.

Even though in principle this repository can support different Debian releases,
currently only Dockerfiles for Debian Bullseye are provided.

In order to build an image, run the commands below from the root directory of
the repository.

```shell
GCC_VERSION=12
DEBIAN_VERSION=bullseye
CONTAINER_IMAGE=ghcr.io/xrplf/ci/gcc:${GCC_VERSION}-${DEBIAN_VERSION}

docker buildx build . \
  --file docker/gcc/Dockerfile.${GCC_VERSION}-${DEBIAN_VERSION} \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
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

However, there should be no need to push the GCC image manually, as it is only
used as a base image for other images in this repository.

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
