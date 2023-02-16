#!/bin/bash

## setup & configure repository
workspaceFolder=$PWD
chmod +x ${workspaceFolder}/script/*

# download corresponding submodules
git submodule update --init --remote

## provision system dependencies
# - libfuse 3+

dependencies="build-essential autoconf libtool pkg-config gcc cmake fuse3 libfuse3-dev libfuse-dev"
# if any issues try: `libfuse3-dev libfuse3-3 fuse` or try searching for fuse3 libraries

sudo apt update && sudo apt upgrade
for i in $dependencies; do sudo apt install -y $i; done
sudo apt update && sudo apt upgrade
sudo apt autoremove

## install vcpkg package manager and dependencies
# https://github.com/grpc/grpc/tree/master/src/cpp#install-using-vcpkg-package
pushd ./dependency/vcpkg
./bootstrap-vcpkg.sh -disableMetrics && ./vcpkg integrate install # >./CMake-script-for-vcpkg.txt
./vcpkg install grpc
popd

##########################################
# Fedora gcc installation
# dnf groupinstall 'Development Tools'
# yum install gcc-c++
# yum install fuse fuse-devel
