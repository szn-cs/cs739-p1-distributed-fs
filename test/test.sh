getFileAttributes() {
  # equivalent to `lstat``
  stat $MOUNTPOINT
  echo "content" >$ROOT/file.txt
  stat $ROOT/file.txt

  # du -b file |cut -f1

  # ls -l file| awk '{print $5}'

  # wc -c file | cut -f1
}

getFileContents() {
  echo "content" >$ROOT/file.txt
  cat $ROOT/file.txt
}

manipulateDirectories() {
  mkdir $ROOT/x
  mkdir $ROOT/x
  rmdir $ROOT/x
  rmdir $ROOT/x
}
