#!/usr/bin/env bash
# See common/run_problem.sh for modes: ./run_tests.sh [asan|tsan|release|bench] [test-filter]
NAME=spsc_queue
DEFAULT_MODES="asan tsan"
EXTRA_LIBS=""
source "$(dirname "${BASH_SOURCE[0]}")/../../common/run_problem.sh"
