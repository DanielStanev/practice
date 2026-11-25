#!/usr/bin/env bash
# Runs the tests against the reference solutions in a temporary copy of the repo.
# Arguments are passed to the top-level run_all.sh (e.g. "dp -- 09").
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf -- "${TMP:?}"' EXIT
tar -C "$ROOT" --exclude=.git --exclude=build -cf - . | tar -C "$TMP" -xf -
for set_dir in "$ROOT"/solutions/*/; do
    set_name="$(basename "$set_dir")"
    cp -r "$set_dir". "$TMP/$set_name/problems/"
done
"$TMP/run_all.sh" "$@"
