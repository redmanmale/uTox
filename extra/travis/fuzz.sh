#!/bin/sh
# Short libFuzzer run against already-built tests/fuzz/* (Clang, ENABLE_FUZZ=ON).
# Coverage-guided; Sanitizers: fuzzer+ASAN+UBSAN.
set -e -u -x

. ./extra/travis/env.sh

if [ -z "${ASAN_OPTIONS+x}" ]; then
    export ASAN_OPTIONS=detect_leaks=0:halt_on_error=1:abort_on_error=1:exitcode=1
fi
if [ -z "${UBSAN_OPTIONS+x}" ]; then
    export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1:abort_on_error=1
fi

runs="${FUZZ_RUNS:-1000}"
timeout="${FUZZ_TIMEOUT:-10}"
mkdir -p tox
for f in tests/fuzz/fuzz_theme tests/fuzz/fuzz_utf8 tests/fuzz/fuzz_commands tests/fuzz/fuzz_chatlog; do
  "$f" -runs="$runs" -timeout="$timeout" -artifact_prefix=tox/fuzz-
done
