getFileAttributes() {
  # equivalent to `lstat``
  stat $MOUNTPOINT
  echo "content" >$SERVER/file.txt
  stat $ROOT/file.txt

  # du -b file |cut -f1

  # ls -l file| awk '{print $5}'

  # wc -c file | cut -f1
}

getFileContents() {
  echo "content" >$SERVER/file.txt
  cat $MOUNTPOINT/file.txt
}

manipulateDirectories() {
  mkdir $MOUNTPOINT/x
  ls $SERVER
  mkdir $MOUNTPOINT/x
  rmdir $MOUNTPOINT/x
  ls $SERVER
  rmdir $MOUNTPOINT/x
}

removeFile() {
  echo "content" >$SERVER/file.txt
  unlink $MOUNTPOINT/file.txt
}

trace() {
  touch file.txt
  strace -s 2000 -o unlink.log unlink file.txt
}
