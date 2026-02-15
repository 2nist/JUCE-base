#!/usr/bin/env bash
set -euo pipefail

CONFIG="${1:-Debug}"
BUILD_DIR="${2:-build_projucer}"
NO_BUILD="${3:-}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ "${NO_BUILD}" != "--no-build" ]]; then
  "${SCRIPT_DIR}/build-projucer.sh" "${CONFIG}" "${BUILD_DIR}"
fi

declare -a candidates=(
  "${BUILD_DIR}/_deps/juce-build/extras/Projucer/Projucer_artefacts/${CONFIG}/Projucer"
  "${BUILD_DIR}/_deps/juce-build/extras/Projucer/Projucer_artefacts/${CONFIG}/Projucer.exe"
  "${BUILD_DIR}/_deps/juce-build/extras/Projucer/Projucer_artefacts/Projucer.app/Contents/MacOS/Projucer"
  "${BUILD_DIR}/_deps/juce-build/extras/Projucer/Projucer_artefacts/Projucer"
)

for exe in "${candidates[@]}"; do
  if [[ -x "${exe}" ]]; then
    exec "${exe}"
  fi
done

echo "Projucer executable not found in ${BUILD_DIR}."
exit 1
