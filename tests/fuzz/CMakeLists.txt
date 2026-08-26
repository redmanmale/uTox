# libFuzzer targets. Built only with Clang and -DENABLE_FUZZ=ON.
# Sanitizers: fuzzer + ASAN + UBSAN (same combo as Linux debug unit tests).

set(UTOX_FUZZ_SANITIZE "-fsanitize=fuzzer,address,undefined")

function(make_fuzz name)
    add_executable(fuzz_${name} fuzz_${name}.c)
    set_target_properties(fuzz_${name} PROPERTIES COMPILE_FLAGS "-Wno-unused-parameter")
    target_compile_options(fuzz_${name} PRIVATE
        ${UTOX_FUZZ_SANITIZE}
        -fno-omit-frame-pointer
        -fno-sanitize-recover=undefined)
    target_link_options(fuzz_${name} PRIVATE ${UTOX_FUZZ_SANITIZE})
    target_include_directories(fuzz_${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    if(ARGN)
        target_link_libraries(fuzz_${name} ${ARGN})
    endif()
endfunction()

make_fuzz(theme utox-test-mock pthread)
make_fuzz(utf8)
make_fuzz(commands utox-test-mock-flist utox-test-mock-domain utox-test-mock pthread)
make_fuzz(chatlog utox-test-mock pthread)
