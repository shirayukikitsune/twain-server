add_executable(gliese-scanner main.cpp external/twain.h application.hpp twain.cpp twain.hpp exception/dsm_exception.hpp http/listener.cpp http/listener.hpp exception/http_exception.hpp)

if (MSVC OR MSYS OR MINGW)
elseif(APPLE)
else()
    target_sources(gliese-scanner PUBLIC application_linux.cpp)
endif()

target_compile_definitions(gliese-scanner PUBLIC LOGURU_WITH_STREAMS=1)
target_include_directories(gliese-scanner PUBLIC ${CMAKE_SOURCE_DIR}/deps/loguru ${CMAKE_SOURCE_DIR}/deps/h2o/include)
target_link_libraries(gliese-scanner PRIVATE nlohmann_json::nlohmann_json
    loguru
    libh2o
    ${Boost_SYSTEM_LIBRARY_RELEASE}
    ${Boost_FILESYSTEM_LIBRARY_RELEASE})
