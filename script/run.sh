#!/bin/bash

fs_mount() {
  mkdir -p /tmp/fs

  sudo ln -s /proc/self/mounts /etc/mtab
  ## make sure it is unmounted
  fusermount -u /tmp/fs
  # or use `umount /tmp/fs`

  ./target/release/unreliablefs /tmp/fs -basedir=/tmp -seed=1618680646 -d

  # (don't add tab in front of content lines)
  cat <<EOF >/tmp/fs/unreliablefs.conf
[errinj_noop]
op_regexp = .*
path_regexp = .*
probability = 30
EOF

  ls -la

  umount /tmp/fs
}
