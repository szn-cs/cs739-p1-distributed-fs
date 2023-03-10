#!/bin/bash

remote_setup() {
  REMOTE=sq089ahy@c220g5-111332.wisc.cloudlab.us

  ssh $REMOTE
  {
    sudo su -
    if ! uname -a | grep -q "microsoft"; then
      chmod -R 777 .
    else
      echo "NOT REMOTE !"
    fi
  }

  # copy over workload files over and run tests
  source ./script/setenv.sh
  scp -rC ./script ./target/release/* ./test/filebench_workloads ./config/unreliablefs.conf $REMOTE:$PROJECT

  # install filebench
  ssh $REMOTE
  {
    sudo su -
    # . ./<myScript>.sh && <function_name> # call filebench function from file
    (source ./script/provision_remote.sh && filebench)
  }

}

filebench() {
  # must be root
  # sudo su -

  # Disable ASLR https://linux-audit.com/linux-aslr-and-kernelrandomize_va_space-setting/
  echo 0 >/proc/sys/kernel/randomize_va_space

  ##### INSTALL filebench command

  pushd .

  # NOTE: tagged release version causes error
  # REPO=https://github.com/filebench/filebench/archive/refs/tags/1.4.9.1.tar.gz
  # mkdir repo_filebench && curl -L $REPO | tar xzC repo_filebench && cd repo_filebench && cd *

  REPO=https://github.com/filebench/filebench/
  mkdir -p repo_filebench && cd repo_filebench && git clone $REPO && cd ./filebench

  libtoolize
  aclocal
  autoheader
  automake --add-missing
  autoconf

  ./configure
  make
  sudo make install
  # mv ./filebench ../../
  popd
  rm -r repo_filebench

  # run tests (check run.sh)
}

# run with $` (source ./script/provision_remote.sh && multiple_remote_upload) `
multiple_remote_upload() {
  source ./script/setenv.sh
  U=sq089ahy

  # Declare an array of string with type
  declare -a REMOTES=(
    "c220g2-010625"
    "c220g2-010624"
    # "c220g2-011121"
    # "c220g2-010623"
    # "c220g1-031120"
    # "c220g2-011126"
  )

  # Iterate the string array using for loop
  for i in ${REMOTES[@]}; do
    scp -rC ./script ./target/release/* ./test/filebench_workloads ./config/unreliablefs.conf $U@$i.wisc.cloudlab.us:$PROJECT
  done
}
