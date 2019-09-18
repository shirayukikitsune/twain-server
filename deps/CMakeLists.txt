add_library(loguru STATIC loguru/loguru.cpp loguru/loguru.hpp)
target_link_libraries(loguru INTERFACE dl)
target_compile_definitions(loguru PUBLIC LOGURU_WITH_STREAMS=1)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)

add_subdirectory(h2o)
