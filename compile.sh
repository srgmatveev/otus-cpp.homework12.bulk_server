#!/bin/sh

TESTS=OFF

mkdir build
cd build

cmake -DBUILD_TEST=${TESTS} ..
cmake --build . -- -j2

case ${TESTS} in
    ON) ctest -j2
esac

cpack .
