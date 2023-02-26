#!/bin/bash

remote_setup() {
  REMOTE=sq089ahy@c220g5-111332.wisc.cloudlab.us
  ssh $REMOTE

  {
    sudo su -
    chmod -R 777 .
    exit # exit root
    exit # exit ssh
  }

  # copy over workload files over and run tests
  source ./script/./script/setenv.sh
  scp -rC ./script ./target/release/* ./test/filebench_workloads ./config/unreliablefs.conf $REMOTE:$PROJECT

  # install filebench
  ssh $REMOTE
  {
    sudo su -
    # . ./<myScript>.sh && <function_name>
    (source ./script/provision_remote.sh && filebench) # call filebench function from file
  }

}

filebench() {
  # must be root
  # sudo su -

  pushd .
  mkdir repo_filebench && curl -L https://github.com/filebench/filebench/archive/refs/tags/1.4.9.1.tar.gz | tar xzC repo_filebench && cd repo_filebench && cd *

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

  # Disable ASLR https://linux-audit.com/linux-aslr-and-kernelrandomize_va_space-setting/
  echo 0 >/proc/sys/kernel/randomize_va_space

  # run tests (check run.sh)
}
