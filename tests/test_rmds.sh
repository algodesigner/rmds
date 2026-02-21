#!/bin/bash

# tests/test_rmds.sh - Automated test suite for rmds

set -e

# Setup test environment
TEST_DIR="test_workspace"
mkdir -p "$TEST_DIR/nest1/nest2"
touch "$TEST_DIR/.DS_Store"
touch "$TEST_DIR/nest1/.DS_Store"
touch "$TEST_DIR/nest1/nest2/.DS_Store"
touch "$TEST_DIR/safe_file.txt"
touch "$TEST_DIR/nest1/other.c"

echo "Running tests..."

# Build if necessary
make > /dev/null

# Run rmds on the test workspace
./rmds "$TEST_DIR" > /dev/null

# Assertions
EXIT_CODE=0

if [ -f "$TEST_DIR/.DS_Store" ] || [ -f "$TEST_DIR/nest1/.DS_Store" ] || [ -f "$TEST_DIR/nest1/nest2/.DS_Store" ]; then
    echo "FAIL: .DS_Store files still exist!"
    EXIT_CODE=1
else
    echo "PASS: All .DS_Store files removed."
fi

if [ ! -f "$TEST_DIR/safe_file.txt" ] || [ ! -f "$TEST_DIR/nest1/other.c" ]; then
    echo "FAIL: Safe files were deleted!"
    EXIT_CODE=1
else
    echo "PASS: Safe files preserved."
fi

# Cleanup
rm -rf "$TEST_DIR"

if [ $EXIT_CODE -eq 0 ]; then
    echo "Test suite PASSED."
else
    echo "Test suite FAILED."
fi

exit $EXIT_CODE
