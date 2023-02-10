# provision system dependencies
# - libfuse 3+

dependencies="build-essential autoconf libtool pkg-config gcc cmake fuse3 libfuse3-dev"

apt update && apt upgrade
for i in $dependencies; do apt install -y $i; done
apt update && apt upgrade
apt autoremove

exit 0

# Fedora gcc installation
# dnf groupinstall 'Development Tools'
# yum install gcc-c++
# yum install fuse fuse-devel

# TODO:  install vcpackage 
vcpackage grpc
