libtoolize
aclocal
autoheader
automake --add-missing
autoconf

curl -L https://github.com/filebench/filebench/archive/refs/tags/1.4.9.1.tar.gz -o install.tgz

tar -xvf ./install.tgz

# cd into directory

./configure
make
sudo make install

echo 0 >/proc/sys/kernel/randomize_va_space

MOUNT_DIR=~/tmp/fsMountpoint

# copy workload files over
