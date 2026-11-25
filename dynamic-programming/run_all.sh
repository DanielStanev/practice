#!/usr/bin/env bash
# Runs every problem's tests and prints a scoreboard.
#
#   ./run_all.sh                  every problem with -O2 + ASan + UBSan
#   ./run_all.sh asan             one mode for all problems (asan | debug | release)
#   ./run_all.sh --compile-only   just check that every problem compiles (stubs included)
#   ./run_all.sh 05 12            only problems whose folder starts with 05 or 12
#
# Full logs go to build/logs/<problem>.log
set -uo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="$ROOT/build/logs"
mkdir -p "$LOG_DIR"

MODE=""
COMPILE_ONLY=0
FILTERS=()
for arg in "$@"; do
    case "$arg" in
        asan|debug|release) MODE="$arg" ;;
        --compile-only) COMPILE_ONLY=1 ;;
        -h|--help) sed -n '2,9p' "$0"; exit 0 ;;
        *) FILTERS+=("$arg") ;;
    esac
done

matches_filter() {
    [[ ${#FILTERS[@]} -eq 0 ]] && return 0
    local f
    for f in "${FILTERS[@]}"; do [[ "$1" == "$f"* ]] && return 0; done
    return 1
}

CXX="${CXX:-g++}"
total=0; passed=0
printf "%-36s %s\n" "PROBLEM" "RESULT"
printf "%-36s %s\n" "-------" "------"
for dir in "$ROOT"/problems/*/; do
    name="$(basename "$dir")"
    matches_filter "$name" || continue
    total=$((total + 1))
    log="$LOG_DIR/$name.log"
    if [[ $COMPILE_ONLY -eq 1 ]]; then
        test_src=$(ls "$dir"test_*.cpp)
        if "$CXX" -std=c++20 -Wall -Wextra -Wpedantic -fsyntax-only \
               -I"$dir" -I"$ROOT/common" "$test_src" >"$log" 2>&1; then
            printf "%-36s %s\n" "$name" "compiles"
            passed=$((passed + 1))
        else
            printf "%-36s %s\n" "$name" "BUILD ERROR (see build/logs/$name.log)"
        fi
        continue
    fi
    if (cd "$dir" && ./run_tests.sh $MODE) >"$log" 2>&1; then
        summary=$(grep -E "^[0-9]+ passed, [0-9]+ failed" "$log" | tail -1)
        printf "%-36s %s\n" "$name" "PASS ($summary)"
        passed=$((passed + 1))
    elif ! grep -q "] running" "$log"; then
        printf "%-36s %s\n" "$name" "BUILD ERROR (see build/logs/$name.log)"
    else
        summary=$(grep -E "^[0-9]+ passed, [0-9]+ failed" "$log" | tail -1)
        [[ -z "$summary" ]] && summary="crashed or timed out"
        grep -qE "Sanitizer|runtime error" "$log" && summary="$summary, sanitizer report"
        printf "%-36s %s\n" "$name" "FAIL ($summary)"
    fi
done
echo
if [[ $COMPILE_ONLY -eq 1 ]]; then
    echo "$passed / $total problems compile"
else
    echo "$passed / $total problems passing"
fi
[[ $passed -eq $total ]]
