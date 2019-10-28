add_executable(twain-dsm-test test.cpp ${CMAKE_SOURCE_DIR}/src/twain/dsm.cpp)

target_link_libraries(twain-dsm-test PUBLIC test-base)

add_test(
    NAME
        twain-dsm-test
    COMMAND
        twain-dsm-test --reporter=spec
)

