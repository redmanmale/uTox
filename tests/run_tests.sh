#!/bin/sh -e

cd "@utoxTESTS_BINARY_DIR@"

# Sanitizer defaults for instrumented Linux builds.
# halt_on_error=1: a bug must fail the test, not print-and-continue.
# Override from the environment if needed.
if [ -z "${ASAN_OPTIONS+x}" ]; then
    export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:abort_on_error=1:detect_stack_use_after_return=1:exitcode=1
fi
if [ -z "${LSAN_OPTIONS+x}" ]; then
    export LSAN_OPTIONS=print_suppressions=0
fi
if [ -z "${UBSAN_OPTIONS+x}" ]; then
    export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1:abort_on_error=1
fi
if [ -z "${TSAN_OPTIONS+x}" ]; then
    export TSAN_OPTIONS=halt_on_error=1:abort_on_error=1
fi

# Valgrind Memcheck (use without ASan/TSan). Discovers the same tests as ctest.
case "${USE_VALGRIND:-OFF}" in
    1|ON|on|TRUE|true|YES|yes)
        VG_OPTS="${VALGRIND_OPTS:---error-exitcode=1 --leak-check=full --track-origins=yes --show-leak-kinds=definite --errors-for-leak-kinds=definite}"
        # Optional project suppressions when present.
        if [ -f "@utoxTESTS_SOURCE_DIR@/valgrind.supp" ]; then
            VG_OPTS="$VG_OPTS --suppressions=@utoxTESTS_SOURCE_DIR@/valgrind.supp"
        fi
        tests=$(ctest -N | sed -n 's/^[[:space:]]*Test #[0-9][0-9]*: //p')
        if [ -z "$tests" ]; then
            echo "No ctest targets found for Valgrind" >&2
            exit 1
        fi
        failed=0
        for t in $tests; do
            rm -rf ./tox
            echo "======== valgrind $t ========"
            # Intentional word-splitting of VG_OPTS (options from us / CI env).
            # shellcheck disable=SC2086
            valgrind $VG_OPTS ./"$t" || failed=1
        done
        exit "$failed"
        ;;
esac

# remove ./tox folder before each test to have clean environment
rm -rf ./tox
ctest -VV
