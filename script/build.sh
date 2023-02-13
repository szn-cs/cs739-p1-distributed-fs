## build through `cmake`
cmake --build ./target/config --verbose --parallel
# or use `make -w -C ./target/config/`

## move binaries from nested builds
mkdir -p ./target/release
cp ./target/config/dependency/unreliablefs/unreliablefs/unreliablefs ./target/release/unreliablefs
cp ./target/config/src/client/client ./target/release/client
cp ./target/config/src/server/server ./target/release/server
# copy grpc example binaries
mkdir -p ./target/release/example-grpc
cp ./target/config/dependency/grpc/examples/cpp/helloworld/greeter* ./target/release/example-grpc

## clean
cmake --build ./target/config --target clean
