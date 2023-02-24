#!/bin/bash

MOUNTPOINT=$(pwd)/tmp/mount
ROOT=$(pwd)/tmp/root
SERVER=$(pwd)/tmp/server
BENCH=$(pwd)/filebench_workloads

run_example() {
  # terminal 1 ###########################################################################################
  ./target/release/example-grpc/greeter_server
  # terminal 2 ###########################################################################################
  ./target/release/example-grpc/greeter_client
}

fs_mount() {

  ######## [terminal instance 1] ##########################################################
  mkdir -p $MOUNTPOINT $ROOT $SERVER

  ## fix some issues probably with WSL2 setup
  # sudo ln -s /proc/self/mounts /etc/mtab
  ## make sure it is unmounted
  fusermount -uz $MOUNTPOINT && fusermount -uz $ROOT
  umount $MOUNTPOINT

  ## unreliable Binary options <https://ligurio.github.io/unreliablefs/unreliablefs.1.html>
  ./target/release/unreliablefs $MOUNTPOINT -basedir=$ROOT -seed=1618680646 -d

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
  cat ./config/unreliablefs.conf $ROOT/unreliablefs.conf

  ls -la $MOUNTPOINT
  pushd $MOUNTPOINT
  echo " " >>file.txt
  popd

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
  ./target/release/server $SERVER
  echo 0
}

client() {
  ./target/release/client
  echo 0
}

remote() {
  REMOTE=sq089ahy@c220g1-030620.wisc.cloudlab.us
  scp -rC ./target/release/* $REMOTE:~/target/release/
  ssh $REMOTE
}

filebench() {
  # copy over workload files over and run tests
  scp -rC ./test/filebench_workloads $REMOTE:~/

  ############ $ REMOTE SHELL ################
  sudo su -

  # cd go back to user directory

  # run python file
  MOUNTPOINT=$(pwd)/tmp/mount
  BENCH=$(pwd)/filebench_workloads
  MOUNT_DIR=$MOUNTPOINT
  pushd ./filebench_workloads/ && python3 set_dir.py $(pwd) && popd

  # run filebench binary
  filebench -f $BENCH/filemicro_create.f
}
