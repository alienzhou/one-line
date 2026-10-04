#!/bin/bash
set -euo pipefail
task_root="$(cd -- "$(dirname -- "$0")/.." && pwd)"
cd "$task_root"
exec python3 tools/fortune_simulator/server.py "$@"
