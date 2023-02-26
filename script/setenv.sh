export MOUNTPOINT=$(pwd)/tmp/mount
export ROOT=$(pwd)/tmp/root
export SERVER=$(pwd)/tmp/server
export CACHE=$(pwd)/tmp/cache
export BENCH=$(pwd)/filebench_workloads
export PROJECT=/root # cloudlab storage mount point

# read from .env file
# # Show env vars
# grep -v '^#' .env
# # Export env vars
# export $(grep -v '^#' .env | xargs)
