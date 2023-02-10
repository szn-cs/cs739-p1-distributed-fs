#!/bin/bash

fs_mount() {

  ######## [terminal instance 1] ##########################################################

  mkdir -p /tmp/fs

  ## fix some issues probably with WSL2 setup
  sudo ln -s /proc/self/mounts /etc/mtab
  ## make sure it is unmounted
  fusermount -u /tmp/fs
  # or use `umount /tmp/fs`

  ## unreliable Binary options <https://ligurio.github.io/unreliablefs/unreliablefs.1.html>
  ./target/release/unreliablefs /tmp/fs -basedir=/tmp -seed=1618680646 -d

  ######## [terminal instance 2] ##########################################################

  ## fault injection options for unreliablefs.conf <https://ligurio.github.io/unreliablefs/unreliablefs.conf.5.html>
  # (don't add tab in front of content lines)
  cat <<EOF >/tmp/fs/unreliablefs.conf
[errinj_noop]
op_regexp = .*
path_regexp = .*
probability = 30
EOF

  ls -la /tmp/fs

  umount /tmp/fs
}
