#!/usr/bin/env bash
set -euo pipefail

# KIO vs Python benchmark suite (robust to spaces in paths)
# Requirements: kio installed on PATH, python3, /usr/bin/time

if ! command -v kio >/dev/null 2>&1; then
  echo "Error: kio not found on PATH. Install with: sudo cmake --install build" >&2
  exit 1
fi
if ! command -v python3 >/dev/null 2>&1; then
  echo "Error: python3 not found on PATH." >&2
  exit 1
fi
if ! command -v /usr/bin/time >/dev/null 2>&1; then
  echo "Error: /usr/bin/time not found." >&2
  exit 1
fi

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

ECHO() { printf "%s\n" "$*"; }
HR() { printf "\n%s\n" "============================================================"; }

ECHO "KIO vs Python Benchmark Suite"
ECHO "Date: $(date)"
ECHO "Machine: $(uname -a)"
ECHO "CPU: $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | sed 's/^ //')"
ECHO "Cores: $(nproc)"
ECHO "Mem: $(grep -m1 MemTotal /proc/meminfo | awk '{print $2" KB"}')"
HR

# Python files
PY_HELLO="$TMP_DIR/hello.py"
cat >"$PY_HELLO" <<'PY'
x = 41 + 1
print(x)
print("kio")
PY

PY_MOD="$TMP_DIR/mod.py"
cat >"$PY_MOD" <<'PY'
greeting = "hello from module"
PY

PY_IMPORT="$TMP_DIR/import_mod.py"
cat >"$PY_IMPORT" <<'PY'
import mod
print(mod.greeting)
PY

PY_SAVELOAD="$TMP_DIR/saveload.py"
cat >"$PY_SAVELOAD" <<'PY'
import json, tempfile, os
obj = {"a": 1, "b": 2, "msg": "hello"}
fd, path = tempfile.mkstemp(suffix=".json")
os.close(fd)
with open(path, 'w') as f:
    json.dump(obj, f)
with open(path, 'r') as f:
    data = json.load(f)
print(data.get('msg', ''))
os.remove(path)
PY

PY_TIME="$TMP_DIR/time_ms.py"
cat >"$PY_TIME" <<'PY'
import time
print(int(time.time() * 1000))
PY

PY_PRINT100="$TMP_DIR/print100.py"
cat >"$PY_PRINT100" <<'PY'
for i in range(1, 101):
    print(f"line {i}")
PY

# KIO files (run from repo root)
KIO_HELLO="examples/hello.kio"
KIO_IMPORT="$TMP_DIR/import_mod.kio"
cat >"$KIO_IMPORT" <<'KIO'
import "examples/mod.kio";
KIO

KIO_SAVELOAD="$TMP_DIR/saveload.kio"
cat >"$KIO_SAVELOAD" <<'KIO'
let a = 1;
let b = 2;
let msg = "hello";
save "/tmp/kio_tmp_state.json";
load "/tmp/kio_tmp_state.json";
print msg;
KIO

KIO_TIME="$TMP_DIR/time_ms.kio"
cat >"$KIO_TIME" <<'KIO'
print sys "time";
KIO

KIO_PRINT100="$TMP_DIR/print100.kio"
{
  for i in $(seq 1 100); do
    echo "print \"line $i\";"
  done
} >"$KIO_PRINT100"

# Run many iterations with clean quoting
run_many() {
  local label="$1"; shift
  local iters="$1"; shift
  local cmd=("$@")
  /usr/bin/time -f "${label} total (${iters} runs): %E" \
    bash -c '
      iters="$1"; shift
      while [ "$iters" -gt 0 ]; do
        "$@" >/dev/null || exit 1
        iters=$((iters-1))
      done
    ' _ "$iters" "${cmd[@]}"
}

ECHO "Startup + trivial print (1000 runs)"
pushd "$ROOT_DIR" >/dev/null
run_many "KIO hello" 1000 kio "$KIO_HELLO"
popd >/dev/null
run_many "Python hello" 1000 python3 "$PY_HELLO"
HR

ECHO "Module import (500 runs)"
PYTHONPATH="$TMP_DIR" run_many "Python import" 500 python3 "$PY_IMPORT"
pushd "$ROOT_DIR" >/dev/null
run_many "KIO import" 500 kio "$KIO_IMPORT"
popd >/dev/null
HR

ECHO "Save/Load (200 runs)"
run_many "Python saveload" 200 python3 "$PY_SAVELOAD"
pushd "$ROOT_DIR" >/dev/null
run_many "KIO saveload" 200 kio "$KIO_SAVELOAD"
popd >/dev/null
HR

ECHO "System time fetch (1000 runs)"
run_many "Python time" 1000 python3 "$PY_TIME"
pushd "$ROOT_DIR" >/dev/null
run_many "KIO time" 1000 kio "$KIO_TIME"
popd >/dev/null
HR

ECHO "Print 100 lines (100 runs)"
run_many "Python print100" 100 python3 "$PY_PRINT100"
pushd "$ROOT_DIR" >/dev/null
run_many "KIO print100" 100 kio "$KIO_PRINT100"
popd >/dev/null
HR

echo "Benchmarks complete."
