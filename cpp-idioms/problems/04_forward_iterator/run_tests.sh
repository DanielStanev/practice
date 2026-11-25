#!/usr/bin/env bash
# See common/run_problem.sh for modes: ./run_tests.sh [asan|release] [test-filter]
NAME=forward_iterator
DEFAULT_MODES="asan"
EXTRA_LIBS=""
source "$(dirname "${BASH_SOURCE[0]}")/../../common/run_problem.sh"
