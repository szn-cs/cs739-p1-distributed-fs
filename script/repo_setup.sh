workspaceFolder=$PWD
chmod +x ${workspaceFolder}/script/* && sudo ${workspaceFolder}/script/dependency.sh
git submodule update --init
