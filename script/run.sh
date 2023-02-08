mount() {
  rm -r ./target && mkdir -p ./target
  cd ./target && mkdir -p fuse_mount_directory
  ./fuse_driver -d -f -s myfsimage.bin ./fuse_mount_directory
}

unmount() {
  # Pressing [Ctrl]+[C] will umount

  # otherwise:
  fusermount -u ./fuse_mount_directory
}
