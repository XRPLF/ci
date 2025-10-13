ARG RHEL_VERSION
FROM registry.access.redhat.com/ubi${RHEL_VERSION}/ubi:latest AS ubi

# This is not inherited from the base image.
ARG RHEL_VERSION
# The RHEL architecture should be either 'x86_64' or 'aarch64'.
ARG RHEL_ARCH

# Enable the repository containing packages we need that are missing from the
# UBI image.
RUN --mount=type=secret,id=RHEL_KEY,env=RHEL_KEY \
    --mount=type=secret,id=RHEL_ORG,env=RHEL_ORG \
    subscription-manager register --activationkey="${RHEL_KEY}" --org="${RHEL_ORG}" && \
    subscription-manager repos --enable=rhel-${RHEL_VERSION}-for-${RHEL_ARCH}-appstream-rpms
