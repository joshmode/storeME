#!/bin/bash

# clean up
rm -f wal.log
make clean
make

echo "test 1: set and retrieve after restart"
./db <<EOF > output.txt
SET mykey myvalue
EXIT
EOF

output=$(cat output.txt)
if [[ "$output" == *"OK"* ]]; then
    echo "set successful"
else
    echo "set failed"
    echo "$output"
    exit 1
fi

./db <<EOF > output.txt
GET mykey
EXIT
EOF

output=$(cat output.txt)
if [[ "$output" == *"myvalue"* ]]; then
    echo "pass: retrieved 'myvalue'"
else
    echo "fail: expected 'myvalue', got:"
    echo "$output"
    exit 1
fi

echo "test 2: delete and verify after restart"
./db <<EOF > output.txt
DELETE mykey
EXIT
EOF

output=$(cat output.txt)
if [[ "$output" == *"OK"* ]]; then
    echo "delete successful"
else
    echo "delete failed"
    echo "$output"
    exit 1
fi

./db <<EOF > output.txt
GET mykey
EXIT
EOF

output=$(cat output.txt)
if [[ "$output" == *"(nil)"* ]]; then
    echo "pass: retrieved '(nil)'"
else
    echo "fail: expected '(nil)', got:"
    echo "$output"
    exit 1
fi

echo "all tests passed"
rm -f wal.log output.txt
