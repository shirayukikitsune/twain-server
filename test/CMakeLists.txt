add_library(test-base INTERFACE)
target_link_libraries(test-base INTERFACE loguru)
target_include_directories(test-base INTERFACE ${CMAKE_SOURCE_DIR}/deps/bandit ${CMAKE_SOURCE_DIR}/deps/loguru ${CMAKE_SOURCE_DIR}/src)
if (NOT (MSVC OR MSYS OR MINGW))
    target_link_libraries(test-base INTERFACE pthread)
endif()

add_subdirectory(twain-dsm-test)
