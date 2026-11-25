#!/usr/bin/env bash
# Runs the scoreboard for one or more problem sets.
#
#   ./run_all.sh                       every set, default modes (ASan+UBSan, plus TSan where relevant)
#   ./run_all.sh dp                    one set: hft | dp | idioms
#   ./run_all.sh hft idioms            several sets
#   ./run_all.sh dp -- 05 12           pass anything after -- to each set's own run_all.sh
#   ./run_all.sh --compile-only        check every problem in every set compiles
#
# Per-set options (after --): a mode (hft/idioms: asan|tsan|release, dp: asan|debug|release),
# --compile-only, or problem-number prefixes. Logs: <set>/build/logs/<problem>.log
set -uo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

declare -A DIRS=([hft]=hft-cpp [dp]=dynamic-programming [idioms]=cpp-idioms)
ORDER=(hft dp idioms)

SETS=(); PASS=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        hft|dp|idioms) SETS+=("$1") ;;
        --compile-only) PASS+=("$1") ;;
        --) shift; PASS+=("$@"); break ;;
        -h|--help) sed -n '2,11p' "$0"; exit 0 ;;
        *) echo "unknown argument '$1' (sets: hft dp idioms; use -- to pass options to a set)" >&2; exit 2 ;;
    esac
    shift
done
[[ ${#SETS[@]} -eq 0 ]] && SETS=("${ORDER[@]}")

status=0
for s in "${SETS[@]}"; do
    echo "################ ${DIRS[$s]} ################"
    "$ROOT/${DIRS[$s]}/run_all.sh" "${PASS[@]}" || status=1
    echo
done
exit $status
