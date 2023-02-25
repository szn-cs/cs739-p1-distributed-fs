MOUNTPOINT=$(pwd)/tmp/mount
ROOT=$(pwd)/tmp/root
SERVER=$(pwd)/tmp/server
CACHE=$(pwd)/tmp/cache
BENCH=$(pwd)/test/filebench_workloads

for f in $BENCH/*.f; do
  echo $f >>$f.log
  rm -rf $ROOT/* $SERVER/* $CACHE/* $MOUNTPOINT/*
  # filebench -f $f >> $f.log
done

########################

filebench() {
  # copy over workload files over and run tests
  scp -rC ./test/filebench_workloads $REMOTE:~/

  ############ $ REMOTE SHELL ################
  sudo su -

  # cd go back to user directory
  # cd /users/<name>/

  # run python file
  MOUNTPOINT=$(pwd)/tmp/mount
  BENCH=$(pwd)/filebench_workloads
  MOUNT_DIR=$MOUNTPOINT
  pushd $BENCH && python3 set_dir.py $MOUNTPOINT/bench && popd

  # run filebench binary
  filebench -f $BENCH/filemicro_create.f
}
