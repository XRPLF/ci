#!/bin/bash

set -ex

# Install Conan dependencies, configure the project, and build the executable.
conan install . --output-folder=build --build=missing
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Run the executable.
./example

# Remove the Conan dependencies.
conan remove -c "*"
cd $(conan config home)
rm -f .conan.db
rm -rf p/
