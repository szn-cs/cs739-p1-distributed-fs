#!/bin/bash

run_example() {
  # terminal 1 ###########################################################################################
  ./target/release/example-grpc/greeter_server
  # terminal 2 ###########################################################################################
  ./target/release/example-grpc/greeter_client
}

fs_mount() {

  ######## [terminal instance 1] ##########################################################
  mkdir -p ./tmp/fs

  ## fix some issues probably with WSL2 setup
  # sudo ln -s /proc/self/mounts /etc/mtab
  ## make sure it is unmounted
  fusermount -uz ./tmp/fsMountpoint && fusermount -uz ./tmp/fsRoot

  # or use `umount ./tmp/fs`
  pushd ./tmp/fsMountpoint
  echo " " >>file.txt
  popd

  ## unreliable Binary options <https://ligurio.github.io/unreliablefs/unreliablefs.1.html>
  ./target/release/unreliablefs $(pwd)/tmp/fsMountpoint -basedir=/tmp/fsRoot -seed=1618680646 -d

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
  cat ./config/unreliablefs.conf ./tmp/fs/unreliablefs.conf

  ls -la ./tmp/fsMountpoint

  umount ./tmp/fsMountpoint
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

server() {
  ./target/release/server
  echo 0
}

client() {
  ./target/release/client
  echo 0
}

remote() {
  scp -rC ./target/release/* sq089ahy@c220g5-120114.wisc.cloudlab.us:~/target/release/
  ssh sq089ahy@c220g5-120114.wisc.cloudlab.us
}
