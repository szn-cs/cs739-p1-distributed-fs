#!/bin/bash
# all command must be run on `Cloudlab.us` with `sudo su -`
# on an isntance of UBUNTU 20.04 with Temporary Filesystem Size of 40 GB mounted to /root
# copy over binaries from ./target/release

server() {
  source ./script/setenv.sh

  ./grpc-server $SERVER
}

fs_mount() {
  # run everything under root
  source ./script/setenv.sh

  ######## [terminal instance 1] ##########################################################
  # rm -rf $MOUNTPOINT/* $ROOT/* $SERVER/* $CACHE/*
  mkdir -p $MOUNTPOINT $ROOT $SERVER $CACHE

  ## unreliable Binary options <https://ligurio.github.io/unreliablefs/unreliablefs.1.html>
  # SERVER_ADDRESS=0.0.0.0:50051
  SERVER_ADDRESS=c220g2-010810.wisc.cloudlab.us:50051
  ./unreliablefs $MOUNTPOINT -basedir=$ROOT -seed=1618680646 -d -serverAddress=$SERVER_ADDRESS

  ######## [terminal instance 2] ##########################################################

  ## fault injection options for unreliablefs.conf <https://ligurio.github.io/unreliablefs/unreliablefs.conf.5.html>
  # (don't add tab in front of content lines)
  #   cat <<EOF >./tmp/fs/unreliablefs.conf
  # [errinj_noop]
  # op_regexp = .*
  # path_regexp = .*
  # probability = 30
  # EOF

  ## use existing config file instead
}

fs_mount_nodebug() {
  source ./script/setenv.sh
  mkdir -p $MOUNTPOINT $ROOT $SERVER $CACHE
  SERVER_ADDRESS=0.0.0.0:50051
  ./unreliablefs $MOUNTPOINT -basedir=$ROOT -serverAddress=$SERVER_ADDRESS
}

fs_config() {
  source ./script/setenv.sh

  ./unreliablefs.conf $ROOT/unreliablefs.conf
}

fs_unmount() {
  source ./script/setenv.sh

  ## fix some issues probably with WSL2 setup
  # sudo ln -s /proc/self/mounts /etc/mtab
  ## make sure it is unmounted
  fusermount -uz $MOUNTPOINT && fusermount -uz $ROOT
  umount $MOUNTPOINT

}

example_run() {
  # terminal 1 ###########################################################################################
  ./target/release/example-grpc/greeter_server
  # terminal 2 ###########################################################################################
  ./target/release/example-grpc/greeter_client
}

example_grpc() {
  ## an example for server & client is located at `./dependency/grpc/examples/cpp/helloworld`
  # https://grpc.io/docs/languages/cpp/quickstart/#build-the-example
  tree -L 2 ./dependency/grpc/examples/cpp/helloworld
  cd ./dependency/grpc/exmples/cpp/helloworld
  # assuming vcpkg manages grpc installation.
  # DOESN'T WORK, SOMETHING TODO WITH THE PATHS
  #cmake -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/../../../../../dependency/vcpkg/scripts/buildsystems/vcpkg.cmake .
}
