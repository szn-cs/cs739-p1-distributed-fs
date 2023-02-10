## build through `cmake` or use `make -w -C ./target/config/`
cmake --build ./target/config --verbose --parallel
mkdir -p ./target/release && cp ./target/config/dependency/unreliablefs/unreliablefs/unreliablefs ./target/release/unreliablefs

## clean
# cmake --build ./target/config --target clean
