add_library(loguru STATIC loguru/loguru.cpp loguru/loguru.hpp)
if (NOT (MSVC OR MSYS OR MINGW))
  target_link_libraries(loguru INTERFACE dl)
endif()
target_compile_definitions(loguru PUBLIC LOGURU_WITH_STREAMS=1)

add_subdirectory(cpprestsdk)
