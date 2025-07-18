#!/bin/bash

set -e  # Exit on any error
set -o pipefail

if [[ "$1" == "--clean" ]]; then
    echo "🧹 Cleaning build directory ..."
    rm -rf build
fi

echo "📁 Creating build directory ..."
mkdir -p build
cd build

echo "🔧 Running CMake configuration ..."
cmake ..

echo "🛠️  Building the project ..."
cmake --build . -j$(nproc)

echo "📂 Copying config and model data ..."
cp ../config.ini .
cp -r ../models . 2>/dev/null || echo "⚠️  models/ folder not found"

echo "✅ Running tests ..."
ctest --output-on-failure

