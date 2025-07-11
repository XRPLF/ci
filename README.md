# CI

Containers for use by CI pipelines in XRPLF projects.

## Build images

Container images created by `debian` `rhel` and `ubuntu` workflows are meant to provide minimum
appropriate build environment for select XRPLF projects, notably [clio](https://github.com/XRPLF/clio)
and [rippled](https://github.com/XRPLF/rippled). These images contain tools required
to build, run unit tests, and package different XRPLF projects. By using the same set of images
for different projects, we are enforcing that XRPLF projects can be built in a wide selection
of environments and using different C++ compilers.

[Ubuntu instructions](docker/ubuntu/README.md)
[Debian instructions](docker/debian/README.md)
[RHEL instructions](docker/rhel/README.md)

## Tools images

Aside from build images, XRPLF projects also use container images with specialized tools, e.g.
to enforce code formatting, run sanitizers, run code coverage tools etc.
The required images are created by workflows starting with `tools-` and ending
with the project name e.g. `tools-rippled`, and are only meant to support specific projects.
These images may also contain a complete C++ build environment, if needed.
