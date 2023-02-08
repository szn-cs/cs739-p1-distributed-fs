# install system dependencies
# - libfuse 3+

dependencies="build-essential pkg-config cmake fuse3 libfuse3-dev"

apt update && apt upgrade
for i in $dependencies; do apt install -y $i; done
apt update && apt upgrade
apt autoremove

exit 0
