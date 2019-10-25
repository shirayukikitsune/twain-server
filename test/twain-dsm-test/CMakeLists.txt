add_executable(twain-dsm-test test.cpp)

add_test(
    NAME
        twain-dsm-test
    COMMAND
        twain-dsm-test --reporter=spec
)

