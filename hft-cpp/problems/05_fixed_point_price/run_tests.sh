#!/usr/bin/env bash
# See common/run_problem.sh for modes: ./run_tests.sh [asan|tsan|release|bench] [test-filter]
NAME=fixed_point
DEFAULT_MODES="asan"
EXTRA_LIBS=""
source "$(dirname "${BASH_SOURCE[0]}")/../../common/run_problem.sh"
