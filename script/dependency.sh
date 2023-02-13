## provision system dependencies
# - libfuse 3+

dependencies="build-essential autoconf libtool pkg-config gcc cmake fuse3 libfuse3-dev libfuse-dev"
# if any issues try: `libfuse3-dev libfuse3-3 fuse` or try searching for fuse3 libraries

apt update && apt upgrade
for i in $dependencies; do apt install -y $i; done
apt update && apt upgrade
apt autoremove

exit 0

## install vcpkg package manager and dependencies
# https://github.com/grpc/grpc/tree/master/src/cpp#install-using-vcpkg-package
cd ./dependency/vcpkg && ./bootstrap-vcpkg.sh -disableMetrics && ./vcpkg integrate install # >./CMake-script-for-vcpkg.txt
./vcpkg install grpc

##########################################
# Fedora gcc installation
# dnf groupinstall 'Development Tools'
# yum install gcc-c++
# yum install fuse fuse-devel
