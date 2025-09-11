#!/usr/bin/env bash
set -euo pipefail

# KIO vs Go benchmark suite (robust to spaces in paths)
# Requirements: kio on PATH, go (golang), /usr/bin/time

if ! command -v kio >/dev/null 2>&1; then
  echo "Error: kio not found on PATH. Install with: sudo cmake --install build" >&2
  exit 1
fi
if ! command -v go >/dev/null 2>&1; then
  echo "Error: go (golang) not found on PATH. Install Go to run this benchmark." >&2
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

ECHO "KIO vs Go Benchmark Suite"
ECHO "Date: $(date)"
ECHO "Machine: $(uname -a)"
ECHO "CPU: $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | sed 's/^ //')"
ECHO "Cores: $(nproc)"
ECHO "Mem: $(grep -m1 MemTotal /proc/meminfo | awk '{print $2" KB"}')"
HR

# Go programs
GO_HELLO_DIR="$TMP_DIR/go_hello"; mkdir -p "$GO_HELLO_DIR"
cat >"$GO_HELLO_DIR/main.go" <<'GO'
package main
import "fmt"
func main(){
    x := 41+1
    fmt.Println(x)
    fmt.Println("kio")
}
GO

GO_IMPORT_DIR="$TMP_DIR/go_import"; mkdir -p "$GO_IMPORT_DIR"
cat >"$GO_IMPORT_DIR/mod.go" <<'GO'
package main
var Greeting = "hello from module"
GO
cat >"$GO_IMPORT_DIR/main.go" <<'GO'
package main
import (
    "fmt"
)
func main(){
    fmt.Println(Greeting)
}
GO

GO_SAVELOAD_DIR="$TMP_DIR/go_saveload"; mkdir -p "$GO_SAVELOAD_DIR"
cat >"$GO_SAVELOAD_DIR/main.go" <<'GO'
package main
import (
    "encoding/json"
    "fmt"
    "os"
)
func main(){
    obj := map[string]interface{}{"a":1, "b":2, "msg":"hello"}
    f, err := os.CreateTemp("", "kio_go_*.json")
    if err!=nil { panic(err) }
    path := f.Name()
    enc := json.NewEncoder(f)
    if err := enc.Encode(obj); err!=nil { panic(err) }
    f.Close()
    data := map[string]interface{}{}
    g, _ := os.Open(path)
    dec := json.NewDecoder(g)
    dec.Decode(&data)
    g.Close()
    os.Remove(path)
    fmt.Println(data["msg"])
}
GO

GO_TIME_DIR="$TMP_DIR/go_time"; mkdir -p "$GO_TIME_DIR"
cat >"$GO_TIME_DIR/main.go" <<'GO'
package main
import (
    "fmt"
    "time"
)
func main(){
    fmt.Println(time.Now().UnixMilli())
}
GO

GO_PRINT100_DIR="$TMP_DIR/go_print100"; mkdir -p "$GO_PRINT100_DIR"
cat >"$GO_PRINT100_DIR/main.go" <<'GO'
package main
import "fmt"
func main(){
    for i:=1; i<=100; i++ { fmt.Printf("line %d\n", i) }
}
GO

# Build Go binaries
( cd "$GO_HELLO_DIR" && go build -o hello_go main.go )
( cd "$GO_IMPORT_DIR" && go build -o import_go main.go mod.go )
( cd "$GO_SAVELOAD_DIR" && go build -o saveload_go main.go )
( cd "$GO_TIME_DIR" && go build -o time_go main.go )
( cd "$GO_PRINT100_DIR" && go build -o print100_go main.go )

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
run_many "Go hello" 1000 "$GO_HELLO_DIR/hello_go"
HR

ECHO "Module import (500 runs)"
pushd "$ROOT_DIR" >/dev/null
run_many "KIO import" 500 kio "$KIO_IMPORT"
popd >/dev/null
run_many "Go import" 500 "$GO_IMPORT_DIR/import_go"
HR

ECHO "Save/Load (200 runs)"
pushd "$ROOT_DIR" >/dev/null
run_many "KIO saveload" 200 kio "$KIO_SAVELOAD"
popd >/dev/null
run_many "Go saveload" 200 "$GO_SAVELOAD_DIR/saveload_go"
HR

ECHO "System time fetch (1000 runs)"
pushd "$ROOT_DIR" >/dev/null
run_many "KIO time" 1000 kio "$KIO_TIME"
popd >/dev/null
run_many "Go time" 1000 "$GO_TIME_DIR/time_go"
HR

ECHO "Print 100 lines (100 runs)"
pushd "$ROOT_DIR" >/dev/null
run_many "KIO print100" 100 kio "$KIO_PRINT100"
popd >/dev/null
run_many "Go print100" 100 "$GO_PRINT100_DIR/print100_go"
HR

echo "Benchmarks complete."
