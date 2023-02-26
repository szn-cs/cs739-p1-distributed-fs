#!/bin/bash
source ./script/setenv.sh

filebench_test() {
  source ./script/setenv.sh
  # run filebench

  # cd go back to user directory
  cd $PROJECT

  # run python file
  MOUNT_DIR=$MOUNTPOINT # seems to be used by set_dir.py
  pushd $BENCH && python3 set_dir.py $MOUNTPOINT/bench && popd

  # run filebench binary
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
    # filebench -f $BENCH/filemicro_create.f
  done

  # TODO: check additional required packages to solve webserver.f workload issue
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
