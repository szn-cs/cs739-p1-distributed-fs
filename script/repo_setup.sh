# configure repository

workspaceFolder=$PWD
chmod +x ${workspaceFolder}/script/* && sudo ${workspaceFolder}/script/dependency.sh

# download corresponding submodules
git submodule update --init

# create make files
cmake -S . -B ./target/release
