# WiscFS [cs739-p1-distributed-fs]

AFS-like Distributed FS project based on <https://github.com/ligurio/unreliablefs>. AFSv1: whole-file caching on local client machine.

- OSTEP book: AFSv1 Protocol Highlights (Early version: caches only files, while directories are kept on the server) - check protobuf interface.

- [P1 Instructions](documentation/CS739.P1.md)
- build (adding AFS protocol based on RPC) on top of an existing FUSE filesystem [unreliablefs](https://github.com/ligurio/unreliablefs)
- CloudLabs

# Setup

- execute `./script/provision.sh` to setup repo and install dependencies
- run script `build.sh` will result in binary files in `./target/release`
- run individual commands (copy & paste) in `run.sh`

# TODO

# client side functions:

- [ ] ls
- [ ] stat
- [ ] rmdir
- [ ] mkdir
- [ ] touch(create)
- [ ] read
- [ ] write
- [ ] unlink

# server side functions:

- [ ] ls
- [ ] stat
- [ ] rmdir
- [ ] mkdir
- [ ] touch(create)
- [ ] read
- [ ] write
- [ ] unlink

# gRPC services:

- [ ] getAttr
- [ ] mkdir
- [ ] rmdir
- [ ] touch(create)
- [ ] read
- [ ] write
- [ ] unlink

# FUSE resources:

- [ ] official specification <https://libfuse.github.io/doxygen/index.html>
  - [ ] <https://github.com/libfuse/libfuse/blob/master/include/fuse.h>
- [ ] FUSE alternative doc <https://www.fsl.cs.stonybrook.edu/docs/fuse/fuse-article-appendices.html>
- [ ] <https://manpages.ubuntu.com/manpages/bionic/man4/fuse.4.html>
- [x] <https://fsgeek.ca/2019/06/18/fuse-file-systems-in-user-space/>
- [ ] \*\* <https://wiki.osdev.org/FUSE>
- [ ] <https://man7.org/linux/man-pages/man4/fuse.4.html>
- [x] <https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/>
  - [ ] documentation & tutorial <https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/html/index.html>
- [x] <https://medium.com/@jain.sm/filesystem-in-userspace-5d1b398b04e>
  - [ ] <https://medium.com/@jain.sm/file-system-in-user-space-example-a63a21236270>
  - [ ] <https://github.com/JulesWang/helloworld-fuse>
- [x] <https://www.cs.cmu.edu/~fp/courses/15213-s07/lectures/15-filesys/index.html>
- [ ] <https://stackoverflow.com/questions/15604191/fuse-detailed-documentation>
- [ ] FUSE Documentation <https://www.cs.hmc.edu/~geoff/classes/hmc.cs135.201109/homework/fuse/fuse_doc.html>
- [ ] Fuse Tutorial <https://www.youtube.com/watch?v=LZCILvr5tUk>
- [ ] Fuse info <https://northstar-www.dartmouth.edu/~richard/WhitePapers/FUSE.html#:~:text=The%20FUSE%20device%20driver%20is,via%20a%20well%20defined%20API.>
- [ ] cpp example FUSE <https://code.google.com/archive/p/fuse-examplefs/source/default/source>
- [ ] <https://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/>
- [ ] Examples:
  - [ ] <https://github.com/rohithvsm/SankalpaFS>
  - [ ] <https://github.com/c5h11oh/DistributedSystems-AFSv1>

# gRPC resources:

- [ ] Basics tutorial <https://grpc.io/docs/languages/cpp/basics/>
- [ ] Github documentation + resources <https://github.com/grpc/grpc/tree/master/src/cpp>
- [ ] <https://github.com/grpc/grpc/tree/master/examples/cpp>

# Protocal Buffers

- [ ] Protocal Buffers <https://developers.google.com/protocol-buffers/docs/overview>

# Resoureces: 

- CPP in C <https://github.com/jacky-wangjj/C_call_Cpp>
- Status cache & File cache <https://pdfs.semanticscholar.org/d25f/9f6b4ec93cffbdcc56f0aa2c7fa0edeaaa25.pdf>
- <https://www.slideshare.net/GiorgioVitiello/andrew-file-system>
- NFS functions <https://docs.oracle.com/cd/E19620-01/805-4448/z4000027624/index.html>
