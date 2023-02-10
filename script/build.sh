## build through `cmake`
cmake --build ./target/config --verbose --parallel
# or use `make -w -C ./target/config/`

## move binaries from nested builds
mkdir -p ./target/release && cp ./target/config/dependency/unreliablefs/unreliablefs/unreliablefs ./target/release/unreliablefs

## clean
# cmake --build ./target/config --target clean
