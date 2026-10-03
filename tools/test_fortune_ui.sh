#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${repo_root}"
lvgl_path="${FORTUNE_LVGL_PATH:-${repo_root}/managed_components/lvgl__lvgl}"
cmake -S tests/fortune_ui_host -B build/fortune-ui-host -G Ninja \
    -D "LVGL_PATH=${lvgl_path}" -D CMAKE_BUILD_TYPE=Release
cmake --build build/fortune-ui-host
mkdir -p build/fortune-preview
build/fortune-ui-host/test_fortune_ui "${repo_root}/build/fortune-preview" "$@"
