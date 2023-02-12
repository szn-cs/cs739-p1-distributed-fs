## provision system dependencies
# - libfuse 3+

dependencies="build-essential autoconf libtool pkg-config gcc cmake fuse3 libfuse3-dev libfuse-dev"

apt update && apt upgrade
for i in $dependencies; do apt install -y $i; done
apt update && apt upgrade
apt autoremove

exit 0

## install vcpkg package manager and dependencies
cd ./dependency/vcpkg && ./bootstrap-vcpkg.sh -disableMetrics && ./vcpkg integrate install # >./CMake-script-for-vcpkg.txt
./vcpkg install grpc

##########################################
# Fedora gcc installation
# dnf groupinstall 'Development Tools'
# yum install gcc-c++
# yum install fuse fuse-devel
