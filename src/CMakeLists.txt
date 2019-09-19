add_executable(gliese-scanner main.cpp external/twain.h application.hpp twain.cpp twain.hpp exception/dsm_exception.hpp http/listener.cpp http/listener.hpp exception/http_exception.hpp)

if (MSVC OR MSYS OR MINGW)
    target_sources(gliese-scanner PUBLIC application_windows.cpp application_windows.hpp)
elseif(APPLE)
else()
    target_sources(gliese-scanner PUBLIC application_linux.cpp application_linux.hpp)
endif()

target_compile_definitions(gliese-scanner PUBLIC LOGURU_WITH_STREAMS=1)
target_include_directories(gliese-scanner PUBLIC ${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/deps/loguru ${CMAKE_SOURCE_DIR}/deps/h2o/include)
target_link_libraries(gliese-scanner PRIVATE cpprest
    loguru
    ${ZLIB_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY})
