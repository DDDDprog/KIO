#!/usr/bin/env bash
# Copyright (c) 2025 Dipanjan Dhar
# SPDX-License-Identifier: GPL-3.0-only

set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cmake -S "$ROOT" -B "$ROOT/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT/build" -j
ctest --test-dir "$ROOT/build" --output-on-failure