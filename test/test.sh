stat() {
  # equivalent to `lstat``
  stat file.txt

  # du -b file |cut -f1

  # ls -l file| awk '{print $5}'

  # wc -c file | cut -f1
}
