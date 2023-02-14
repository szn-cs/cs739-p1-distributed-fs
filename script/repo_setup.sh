# configure repository

workspaceFolder=$PWD
chmod +x ${workspaceFolder}/script/* && sudo ${workspaceFolder}/script/dependency.sh

# download corresponding submodules
git submodule update --init

# vcpkg install grpc
cd ./dependency/vcpkg && ./bootstrap-vcpkg.sh -disableMetrics && ./vcpkg integrate install # >./CMake-script-for-vcpkg.txt
./vcpkg install grpc
cd ../..

# create make files
cmake -S . -B ./target/config
