#!/bin/bash
# Build the node target and run every test.
set -e
cd "$(dirname "$0")/.."
./build.sh node
node tests/compare_screenshot.js
node tests/interact.js
