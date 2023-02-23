MOUNTPOINT=$(pwd)/tmp/mount
ROOT=$(pwd)/tmp/root
SERVER=$(pwd)/tmp/server

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

getFileContents() {
  echo "content" >$SERVER/file.txt
  cat $MOUNTPOINT/file.txt
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

removeFile() {
  echo "content" >$SERVER/file.txt
  unlink $MOUNTPOINT/file.txt
}

trace() {
  touch file.txt
  strace -s 2000 -o unlink.log unlink file.txt
}
