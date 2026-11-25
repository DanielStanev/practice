#!/usr/bin/env bash
# See common/run_problem.sh for modes: ./run_tests.sh [asan|release] [test-filter]
NAME=lifetime
DEFAULT_MODES="asan"
EXTRA_LIBS=""
# Catch references to dead stack frames as well as dead heap objects.
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_stack_use_after_return=1}"
source "$(dirname "${BASH_SOURCE[0]}")/../../common/run_problem.sh"
