#!/usr/bin/env bash
set -euo pipefail

echo "Checking JUCE CMake toolchain requirements (macOS/Linux)..."

missing=0
for tool in git cmake; do
  if command -v "${tool}" >/dev/null 2>&1; then
    echo "${tool}: OK"
  else
    echo "${tool}: MISSING"
    missing=1
  fi
done

if command -v ninja >/dev/null 2>&1; then
  echo "ninja: OK"
else
  echo "ninja: MISSING (recommended for fast builds)"
fi

if command -v clang++ >/dev/null 2>&1 || command -v g++ >/dev/null 2>&1; then
  echo "c++ compiler: OK"
else
  echo "c++ compiler: MISSING"
  missing=1
fi

if [[ "${missing}" -ne 0 ]]; then
  echo "Install missing tools, then re-run this script."
  exit 1
fi

echo "Toolchain check completed."
