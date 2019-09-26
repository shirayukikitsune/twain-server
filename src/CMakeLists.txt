if (MSVC OR MSYS OR MINGW)
    add_executable(gliese-scanner application_windows.cpp application_windows.hpp)
    target_compile_definitions(gliese-scanner PUBLIC UNICODE=1)
# elseif(APPLE)
else()
    add_executable(gliese-scanner application_linux.cpp application_linux.hpp)
endif()

if (APPLE)
    find_library(TWAIN_LIBRARY TWAIN)
    mark_as_advanced(TWAIN_LIBRARY)
    target_link_libraries(gliese-scanner PRIVATE ${TWAIN_LIBRARY})
endif()

target_sources(gliese-scanner PUBLIC
        external/twain.h
        application.cpp application.hpp
        twain.cpp twain.hpp
        exception/dsm_exception.hpp
        exception/http_exception.hpp
        http/listener.cpp http/listener.hpp
        http/session.cpp http/session.hpp
        http/router.cpp http/router.hpp
        http/handler/devices.cpp http/handler/devices.hpp
        http/handler/handler.hpp
        http/handler/handlers.cpp http/handler/handlers.hpp
        http/handler/scan.cpp http/handler/scan.hpp
        http/handler/status.cpp
        twain/transfer.cpp twain/transfer.hpp
        twain/memory_transfer.cpp twain/memory_transfer.hpp
        twain/native_transfer.cpp twain/native_transfer.hpp)

target_compile_definitions(gliese-scanner PUBLIC LOGURU_WITH_STREAMS=1)
target_include_directories(gliese-scanner PUBLIC ${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/deps/loguru)
target_link_libraries(gliese-scanner PRIVATE nlohmann_json::nlohmann_json
    loguru
    ${ZLIB_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY})
