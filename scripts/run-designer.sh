#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build_unix}"

declare -a candidates=(
  "${BUILD_DIR}/DesignerApp_artefacts/Debug/TwoNist Designer"
  "${BUILD_DIR}/DesignerApp_artefacts/Release/TwoNist Designer"
  "${BUILD_DIR}/DesignerApp_artefacts/TwoNist Designer"
  "${BUILD_DIR}/DesignerApp_artefacts/Debug/TwoNist Designer.app/Contents/MacOS/TwoNist Designer"
  "${BUILD_DIR}/DesignerApp_artefacts/Release/TwoNist Designer.app/Contents/MacOS/TwoNist Designer"
  "${BUILD_DIR}/DesignerApp_artefacts/TwoNist Designer.app/Contents/MacOS/TwoNist Designer"
)

for exe in "${candidates[@]}"; do
  if [[ -x "${exe}" ]]; then
    exec "${exe}"
  fi
done

echo "Designer executable not found in ${BUILD_DIR}. Build first."
exit 1
