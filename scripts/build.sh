#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=common.sh
source "${SCRIPT_DIR}/common.sh"

CONFIG="${1:-Debug}"
BUILD_DIR="${2:-build_unix}"

run_cmake --build "${BUILD_DIR}" --config "${CONFIG}"
