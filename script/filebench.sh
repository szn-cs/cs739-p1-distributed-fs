MOUNTPOINT=$(pwd)/tmp/mount
ROOT=$(pwd)/tmp/root
SERVER=$(pwd)/tmp/server
CACHE=$(pwd)/tmp/cache
BENCH=$(pwd)/filebench_workloads

for f in $BENCH/*.f; do
  C='\033[1;36m'
  NC='\033[0m' # No Color
  echo -e "${C}Running filebench: ${f}${NC}"
  echo $f >>$f.log
  if [[ (! $ROOT) && (! $SERVER) && (! $CACHE) && (! $MOUNTPOINT) ]]; then
    trap "exit" 1
  fi
  rm -rf $ROOT/* $SERVER/* $CACHE/* $MOUNTPOINT/*
  # filebench -f $f >>$f.log
  filebench -f $f 2>&1 | tee -a $f.log
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
  MOUNT_DIR=$MOUNTPOINT
  pushd $BENCH && python3 set_dir.py $MOUNTPOINT/bench && popd

  # run filebench binary
  filebench -f $BENCH/filemicro_create.f
}
