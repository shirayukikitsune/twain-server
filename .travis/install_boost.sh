#!/usr/bin/env bash

curl -L https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.tar.bz2 | tar xj

cd boost_1_71_0
./bootstrap.sh
if [[ "$TRAVIS_OS_NAME" = "linux" || "$TRAVIS_OS_NAME" = "osx" ]]; then
  sudo ./b2 --with-filesystem --with-system install
else
  ./b2 --with-date_time --with-filesystem --with-system --build-type=complete install toolset=msvc
fi
