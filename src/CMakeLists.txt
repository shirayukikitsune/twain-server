add_executable(gliese-scanner
        external/twain.h
        application.cpp application.hpp
        twain.cpp twain.hpp
        exception/dsm_exception.hpp
        exception/http_exception.hpp
        http/listener.cpp http/listener.hpp
        http/router.cpp http/router.hpp
        http/handler/devices.cpp
		http/handler/handler.hpp
        http/handler/handlers.cpp http/handler/handlers.hpp
        http/handler/scan.cpp
        http/handler/status.cpp
        twain/transfer.hpp
        twain/memory_transfer.cpp twain/memory_transfer.hpp)

if (MSVC OR MSYS OR MINGW)
    target_sources(gliese-scanner PUBLIC application_windows.cpp application_windows.hpp)
# elseif(APPLE)
else()
    target_sources(gliese-scanner PUBLIC application_linux.cpp application_linux.hpp)
endif()

target_compile_definitions(gliese-scanner PUBLIC LOGURU_WITH_STREAMS=1)
target_include_directories(gliese-scanner PUBLIC ${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/deps/loguru)
target_link_libraries(gliese-scanner PRIVATE nlohmann_json::nlohmann_json
    loguru
    ${ZLIB_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY})
