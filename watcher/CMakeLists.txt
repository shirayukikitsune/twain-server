add_executable(twain-server-watcher application.cpp process.cpp process.h)
target_compile_definitions(twain-server-watcher PUBLIC LOGURU_WITH_STREAMS=1 UNICODE=1)
target_link_libraries(twain-server-watcher PRIVATE loguru)
target_include_directories(twain-server-watcher PUBLIC ${CMAKE_SOURCE_DIR}/deps/loguru)
