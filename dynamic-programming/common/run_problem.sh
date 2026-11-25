#!/usr/bin/env bash
# Shared build-and-run logic for every problem. Each problem's run_tests.sh
# sets NAME and sources this file.
#
# Usage (from a problem folder):
#   ./run_tests.sh                 -O2 + AddressSanitizer + UndefinedBehaviorSanitizer (default)
#   ./run_tests.sh asan [filter]   same, only tests whose name contains filter
#   ./run_tests.sh release [filt]  -O3 -march=native, no sanitizers (to feel the real speed)
#   ./run_tests.sh debug [filter]  -O0 -g + sanitizers, nicest for gdb/lldb
#
# Environment: CXX (default g++), CXXFLAGS (appended), TIMEOUT seconds (default 120).
set -euo pipefail

: "${NAME:?run_tests.sh must set NAME}"

PROBLEM_DIR="$(cd "$(dirname "${BASH_SOURCE[1]}")" && pwd)"
COMMON_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CXX="${CXX:-g++}"
TIMEOUT="${TIMEOUT:-120}"
BUILD_DIR="$PROBLEM_DIR/build"
mkdir -p "$BUILD_DIR"

BASE_FLAGS=(-std=c++20 -Wall -Wextra -Wpedantic -g -I"$PROBLEM_DIR" -I"$COMMON_DIR")
SAN="-fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=undefined"

mode_flags() {
    case "$1" in
        asan)    echo "-O2 $SAN" ;;
        debug)   echo "-O0 $SAN" ;;
        release) echo "-O3 -march=native -DNDEBUG" ;;
        *) echo "unknown mode '$1' (expected asan|debug|release)" >&2; return 1 ;;
    esac
}

run_with_timeout() {
    if command -v timeout >/dev/null 2>&1; then timeout "$TIMEOUT" "$@"; else "$@"; fi
}

run_mode() {
    local mode="$1"; shift
    local src="$PROBLEM_DIR/test_${NAME}.cpp"
    local out="$BUILD_DIR/${NAME}_${mode}"
    local flags
    flags="$(mode_flags "$mode")"
    echo "== $NAME [$mode] compiling"
    # shellcheck disable=SC2086
    "$CXX" "${BASE_FLAGS[@]}" $flags ${CXXFLAGS:-} "$src" -o "$out"
    echo "== $NAME [$mode] running"
    local env_prefix=()
    [[ "$mode" != release ]] && env_prefix=(env ASAN_OPTIONS="detect_leaks=1:abort_on_error=0" UBSAN_OPTIONS="print_stacktrace=1")
    local status=0
    run_with_timeout "${env_prefix[@]}" "$out" "$@" || status=$?
    if [[ $status -eq 124 ]]; then
        echo "== $NAME [$mode] TIMED OUT after ${TIMEOUT}s: your solution is probably too slow (check the complexity target in README.md)"
    fi
    return $status
}

if [[ $# -gt 0 && "$1" =~ ^(asan|debug|release)$ ]]; then
    mode="$1"; shift
    run_mode "$mode" "$@"
else
    run_mode asan "$@"
fi
