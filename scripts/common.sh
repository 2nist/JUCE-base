#!/usr/bin/env bash
set -euo pipefail

repo_root() {
  local script_dir
  script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
  cd "${script_dir}/.." && pwd
}

run_cmake() {
  (cd "$(repo_root)" && cmake "$@")
}

run_ctest() {
  (cd "$(repo_root)" && ctest "$@")
}
