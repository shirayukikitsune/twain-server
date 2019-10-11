#!/usr/bin/env bash

curl -L https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.tar.bz2 | tar xj

cd boost_1_71_0
./bootstrap.sh
./b2 --with-filesystem --with-system install
