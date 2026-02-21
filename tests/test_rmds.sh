#!/bin/bash

# tests/test_rmds.sh - Automated test suite for rmds

set -e

# Setup test environment
TEST_DIR="test_workspace"

setup_test_dir() {
    rm -rf "$TEST_DIR"
    mkdir -p "$TEST_DIR/nest1/nest2"
    touch "$TEST_DIR/.DS_Store"
    touch "$TEST_DIR/nest1/.DS_Store"
    touch "$TEST_DIR/nest1/nest2/.DS_Store"
    touch "$TEST_DIR/safe_file.txt"
    touch "$TEST_DIR/nest1/other.c"
}

# Build
make > /dev/null

echo "Running tests..."

# 1. Test Help Flag
echo -n "Test 1: Help flag... "
./rmds --help | grep -q "Usage:"
echo "PASS"

# 2. Test Dry Run
setup_test_dir
echo -n "Test 2: Dry run... "
OUTPUT=$(./rmds --dry-run "$TEST_DIR")
if [ -f "$TEST_DIR/.DS_Store" ] && echo "$OUTPUT" | grep -q "(dry-run) Would delete"; then
    echo "PASS"
else
    echo "FAIL: Files deleted or output missing"
    exit 1
fi

# 3. Test Quiet Mode
setup_test_dir
echo -n "Test 3: Quiet mode... "
OUTPUT=$(./rmds --quiet "$TEST_DIR")
if [ -z "$OUTPUT" ] && [ ! -f "$TEST_DIR/.DS_Store" ]; then
    echo "PASS"
else
    echo "FAIL: Output produced or files not deleted"
    echo "Output: $OUTPUT"
    exit 1
fi

# 4. Test Basic Recursion (Existing)
setup_test_dir
echo -n "Test 4: Basic recursion... "
./rmds "$TEST_DIR" > /dev/null
if [ ! -f "$TEST_DIR/.DS_Store" ] && [ -f "$TEST_DIR/safe_file.txt" ]; then
    echo "PASS"
else
    echo "FAIL: Files not deleted correctly"
    exit 1
fi

# 5. Test Verbose Mode
setup_test_dir
echo -n "Test 5: Verbose mode... "
OUTPUT=$(./rmds --verbose "$TEST_DIR")
if echo "$OUTPUT" | grep -q "Scanning: $TEST_DIR/nest1"; then
    echo "PASS"
else
    echo "FAIL: Verbose output missing scanning info"
    exit 1
fi

# 6. Test Max Depth
setup_test_dir
echo -n "Test 6: Max depth (level 1)... "
./rmds --max-depth 1 "$TEST_DIR" > /dev/null
# Root (level 0) and nest1 (level 1) should be cleared, nest2 (level 2) should NOT.
if [ ! -f "$TEST_DIR/.DS_Store" ] && [ ! -f "$TEST_DIR/nest1/.DS_Store" ] && [ -f "$TEST_DIR/nest1/nest2/.DS_Store" ]; then
    echo "PASS"
else
    echo "FAIL: Max depth not respected"
    exit 1
fi

# Cleanup
rm -rf "$TEST_DIR"

echo "All tests PASSED."
