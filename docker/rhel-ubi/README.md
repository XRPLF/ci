## RHEL-UBI: A helper Docker image for RHEL

We are providing a helper Docker image for building `rippled` on Red Hat
Enterprise Linux (RHEL) systems. The image is based on the Universal Base Image
(UBI) provided by Red Hat, which is freely available to everyone. However, they
contain a subset of RHEL content, and for our purposes some content is missing.

The helper Docker image adds back a RHEL repo that contains the packages we need
to build and sign an .rpm installer for `rippled`. As credentials are needed to
add the repo, which will not be available in forks, this helper image will
therefore allow external contributors to modify our RHEL image without needing
access to the credentials.

### Obtaining Red Hat credentials

To be able to add the RHEL repo to the UBI image, an activation key is needed,
which can be obtained free of charge. First you need to register for a Developer
account [here](https://developers.redhat.com), and then you can create an activation key
[here](https://console.redhat.com/insights/connector/activation-keys). On that
same page you will find your organization ID.

### Building the Docker image

In order to build an image, run the commands below from the root directory of
the repository.

```shell
export RHEL_KEY=<value-of-rhel-key>
export RHEL_ORG=<value-of-rhel-org>
RHEL_ARCH=<x86_64 or aarch64>
RHEL_VERSION=8
CONTAINER_IMAGE=ghcr.io/xrplf/ci/rhel-${RHEL_VERSION}:ubi

docker buildx build . \
  --file docker/rhel-ubi/Dockerfile \
  --build-arg BUILDKIT_DOCKERFILE_CHECK=skip=InvalidDefaultArgInFrom \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --secret id=RHEL_KEY,env=RHEL_KEY \
  --secret id=RHEL_ORG,env=RHEL_ORG \
  --build-arg RHEL_ARCH=${RHEL_ARCH} \
  --build-arg RHEL_VERSION=${RHEL_VERSION} \
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

However, there should be no need to push the RHEL UBI image manually, as it is
only used as a base image for other images in this repository.

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
