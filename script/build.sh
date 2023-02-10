## build through `cmake` or use `make -w -C ./target/release/`
cmake --build ./target/release --verbose --parallel
cp ./target/release/dependency/unreliablefs/unreliablefs/unreliablefs ./target/release/unreliablefs

## clean
# cmake --build ./target/release --target clean
