#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=common.sh
source "${SCRIPT_DIR}/common.sh"

BUILD_DIR="${1:-build_unix}"
CONFIG="${2:-Debug}"

run_ctest --test-dir "${BUILD_DIR}" --build-config "${CONFIG}" --output-on-failure
