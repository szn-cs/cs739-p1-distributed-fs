# create make files &
# build through `cmake`  or use `make -w -C ./target/config/`
cmake -S . -B ./target/config && cmake --build ./target/config --verbose --parallel
## move binaries from nested builds
mkdir -p ./target/release
cp ./target/config/src/unreliablefs ./target/release/unreliablefs
cp ./target/config/src/grpc-server ./target/release/grpc-server

# copy grpc example binaries
example() {
  mkdir -p ./target/release/example-grpc
  cp ./target/config/dependency/grpc/examples/cpp/helloworld/greeter* ./target/release/example-grpc
}

test() {
  gcc -Wall ./test/testSystemCall.c -o ./target/testSystemCall
}

# original implementation build
originalUnreliablefs() {
  pushd ./dependency/unreliablefs
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build --parallel
  ls ./build/unreliable/unreliablefs
  popd
}

## clean
# cmake --build ./target/config --target clean

################
# Build comments from unreliablefs
# cmake libc-dev build-essential fuse libfuse-dev
# apt-get install -y cmake libc-dev build-essential fuse libfuse-dev mandoc python3-pytest fio
# build_script:
# - mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug ..
# - make

# test_script:
# - make -C build pytest
