#!/bin/bash

# run with $` (source ./script/test.sh && filebench_test) `
filebench_test() {
  source ./script/setenv.sh
  # run filebench

  # cd go back to user directory
  pushd $PROJECT

  # run python file
  MOUNT_DIR=$MOUNTPOINT # seems to be used by set_dir.py
  pushd $BENCH && python3 set_dir.py $MOUNTPOINT/bench && popd

  # run filebench binary
  for f in $BENCH/*.f; do
    C='\033[1;36m'
    NC='\033[0m' # No Color
    echo -e "\n\n\n\n${C}Running filebench: ${f}${NC}"
    if [[ (! $ROOT) && (! $SERVER) && (! $CACHE) && (! $MOUNTPOINT) ]]; then
      trap "exit" 1
    fi

    // TODO: For some reason subsequent tests are faced with stale data in the mounted point which causes failures.
    rm -rf $MOUNTPOINT/* $ROOT/* $SERVER/* $CACHE/*

    read -p "Run this workload? (Y/y for yes)" -n 1 -r
    echo # (optional) move to a new line
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      echo $f >>$f.log
      # redirect to files
      # filebench -f $f >>$f.log

      # redirect to file and print to stdout while testing
      filebench -f $f 2>&1 | tee -a $f.log

      # filebench -f $BENCH/filemicro_create.f
    fi

  done

  mkdir -p results
  mv $BENCH/*.log ./results

  popd
}

# run with $` (source ./script/test.sh && filebench_retrieveResults) `
filebench_retrieveResults() {
  N=bundle-1
  U=sq089ahy

  # Declare an array of string with type
  declare -a REMOTES=(
    # running togeter
    # "c220g2-010625" # client
    # "c220g2-010624" # server

    # "c220g2-011121" # runs locally in background without console output

    # "c220g2-010623" # running local filebench with console ouput

    # # running togher
    # "c220g1-031120" # client
    # "c220g2-011126" # server

    "c220g5-120103"
    "c220g5-110909"
    "c220g5-110912"

  )

  # Iterate the string array using for loop
  for i in ${REMOTES[@]}; do
    mkdir -p ./results/$N/output-$i
    scp -rC $U@$i.wisc.cloudlab.us:$PROJECT/results ./results/$N/output-$i
  done
}

other() {
  ls -la $MOUNTPOINT
  pushd $MOUNTPOINT
  echo " " >>file.txt
  popd
}

trace() {
  touch file.txt
  strace -s 2000 -o unlink.log unlink file.txt
}

attribute() {
  # equivalent to `lstat``
  stat $MOUNTPOINT
  echo "content" >$SERVER/file.txt
  mkdir -p $SERVER/x/y/z/
  echo "content" >$SERVER/x/y/z/file.txt
  stat $MOUNTPOINT/file.txt
  stat $MOUNTPOINT/x/y/z/file.txt

  # du -b file |cut -f1

  # ls -l file| awk '{print $5}'

  # wc -c file | cut -f1
}

file() {
  echo "content" >$SERVER/file.txt
  cat $MOUNTPOINT/file.txt

  echo "content" >$SERVER/file.txt
  unlink $MOUNTPOINT/file.txt

  echo "content" >$SERVER/file.txt
  cat $SERVER/file.txt

}

directory() {
  mkdir $MOUNTPOINT/x
  ls $SERVER

  mkdir $MOUNTPOINT/x

  rmdir $MOUNTPOINT/x

  ls $SERVER
  rmdir $MOUNTPOINT/x

  mkdir $MOUNTPOINT/x
  mkdir $MOUNTPOINT/x/y
  ls $SERVER
  stat $SERVER/x/y

  # mkdir -p: triggers
  # cppWrapper_getattr
  # cppWrapper_opendir
  # cppWrapper_access
  # cppWrapper_releasedir
  # cppWrapper_mkdir
  mkdir -p $MOUNTPOINT/x/y/z/t
  stat $SERVER/x/y/z/t

  rmdir $MOUNTPOINT/x/y/z # error
  rmdir $MOUNTPOINT/x/y/z/t
  rmdir $MOUNTPOINT/x/y/z/
  rmdir $MOUNTPOINT/x/y/
  rmdir $MOUNTPOINT/x/

  #### readdir
  mkdir -p $MOUNTPOINT/x/y/z/t
  ls $MOUNTPOINT
  ls $MOUNTPOINT/x
  ls $MOUNTPOINT/m/t/q

}
