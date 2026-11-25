#!/usr/bin/env bash
# Shared build-and-run logic for every problem. Each problem's run_tests.sh
# sets NAME (and optionally DEFAULT_MODES / EXTRA_LIBS) and sources this file.
#
# Usage (from a problem folder):
#   ./run_tests.sh                 default modes for the problem (asan, plus tsan for concurrency problems)
#   ./run_tests.sh asan [filter]   -O2 + AddressSanitizer + UndefinedBehaviorSanitizer
#   ./run_tests.sh tsan [filter]   -O2 + ThreadSanitizer
#   ./run_tests.sh release         -O3 -march=native, no sanitizers
#   ./run_tests.sh bench           build and run bench_<name>.cpp (if the problem has one)
#
# Environment: CXX (default g++), CXXFLAGS (appended), TIMEOUT seconds (default 180).
set -euo pipefail

: "${NAME:?run_tests.sh must set NAME}"
: "${DEFAULT_MODES:=asan}"
: "${EXTRA_LIBS:=}"

PROBLEM_DIR="$(cd "$(dirname "${BASH_SOURCE[1]}")" && pwd)"
COMMON_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CXX="${CXX:-g++}"
TIMEOUT="${TIMEOUT:-180}"
BUILD_DIR="$PROBLEM_DIR/build"
mkdir -p "$BUILD_DIR"

# GCC warns that TSan does not model atomic_thread_fence; Clang has no such flag.
TSAN_EXTRA=""
"$CXX" --version 2>/dev/null | grep -qi clang || TSAN_EXTRA="-Wno-tsan"

BASE_FLAGS=(-std=c++20 -Wall -Wextra -Wpedantic -g -pthread -I"$PROBLEM_DIR" -I"$COMMON_DIR")

mode_flags() {
    case "$1" in
        asan)    echo "-O2 -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=undefined" ;;
        tsan)    echo "-O2 -fsanitize=thread $TSAN_EXTRA" ;;
        release) echo "-O3 -march=native -DNDEBUG" ;;
        bench)   echo "-O3 -march=native -DNDEBUG" ;;
        *) echo "unknown mode '$1' (expected asan|tsan|release|bench)" >&2; return 1 ;;
    esac
}

run_with_timeout() {
    if command -v timeout >/dev/null 2>&1; then timeout "$TIMEOUT" "$@"; else "$@"; fi
}

run_mode() {
    local mode="$1"; shift
    local src="$PROBLEM_DIR/test_${NAME}.cpp"
    [[ "$mode" == bench ]] && src="$PROBLEM_DIR/bench_${NAME}.cpp"
    if [[ ! -f "$src" ]]; then
        echo "== $NAME: no $(basename "$src"), skipping $mode"
        return 0
    fi
    local out="$BUILD_DIR/${NAME}_${mode}"
    local flags
    flags="$(mode_flags "$mode")"
    echo "== $NAME [$mode] compiling"
    # shellcheck disable=SC2086
    "$CXX" "${BASE_FLAGS[@]}" $flags ${CXXFLAGS:-} "$src" -o "$out" $EXTRA_LIBS
    echo "== $NAME [$mode] running"
    local env_prefix=()
    [[ "$mode" == asan ]] && env_prefix=(env ASAN_OPTIONS="detect_leaks=1:abort_on_error=0" UBSAN_OPTIONS="print_stacktrace=1")
    [[ "$mode" == tsan ]] && env_prefix=(env TSAN_OPTIONS="halt_on_error=1:second_deadlock_stack=1")
    run_with_timeout "${env_prefix[@]}" "$out" "$@"
}

if [[ $# -gt 0 && "$1" =~ ^(asan|tsan|release|bench)$ ]]; then
    mode="$1"; shift
    run_mode "$mode" "$@"
else
    status=0
    for mode in $DEFAULT_MODES; do
        run_mode "$mode" "$@" || status=1
    done
    exit $status
fi
