#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=common.sh
source "${SCRIPT_DIR}/common.sh"

CONFIG="${1:-Debug}"
BUILD_DIR="${2:-build_projucer}"
GENERATOR="${3:-Ninja}"

"${SCRIPT_DIR}/configure-projucer.sh" "${GENERATOR}" "${BUILD_DIR}" "${CONFIG}"
run_cmake --build "${BUILD_DIR}" --config "${CONFIG}" --target Projucer
