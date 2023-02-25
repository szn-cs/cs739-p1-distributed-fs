MOUNTPOINT=$(pwd)/tmp/mount
ROOT=$(pwd)/tmp/root
SERVER=$(pwd)/tmp/server
CACHE=$(pwd)/tmp/cache
BENCH=$(pwd)/test/filebench_workloads

for f in $BENCH/*.f
do
echo $f >> $f.log
rm -rf $ROOT/* $SERVER/* $CACHE/* $MOUNTPOINT/*
# filebench -f $f >> $f.log
done