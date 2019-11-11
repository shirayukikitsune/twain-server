if (MSVC OR MSYS OR MINGW)
  if (APPLICATION_TYPE STREQUAL "Window")
    add_executable(twain-server WIN32 application_windows_native.cpp)
  elseif (APPLICATION_TYPE STREQUAL "Service")
    add_executable(twain-server application_windows_service.cpp)
  else ()
    add_executable(twain-server application_windows_console.cpp)
  endif()
    
  target_sources(twain-server PUBLIC application_windows.cpp application_windows.hpp twain/native_transfer_windows.cpp)
  target_compile_definitions(twain-server PUBLIC UNICODE=1 WIN32_LEAN_AND_MEAN=1)
else()
  add_executable(twain-server application_linux.cpp application_linux.hpp 
          twain/native_transfer_linux.cpp)
  target_link_libraries(twain-server PRIVATE pthread ${FREEIMAGE_LIBRARIES})
endif()

# Silence a few warnings on MSVC
if (MSVC)
  target_compile_definitions(twain-server PUBLIC _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS BOOST_ALL_NO_LIB)
endif()

# In OSX, TWAIN library is a framework that must be statically linked
if (APPLE)
    find_library(TWAIN_LIBRARY TWAIN)
    mark_as_advanced(TWAIN_LIBRARY)
    target_link_libraries(twain-server PRIVATE ${TWAIN_LIBRARY})
endif()

target_sources(twain-server PUBLIC
        external/twain.h
        application.cpp application.hpp
        twain.cpp twain.hpp
        exception/http_exception.hpp
        http/listener.cpp http/listener.hpp
        http/session.cpp http/session.hpp
        http/router.cpp http/router.hpp
        http/handler/devices.cpp http/handler/devices.hpp
        http/handler/handler.hpp
        http/handler/scan.cpp http/handler/scan.hpp
        http/handler/status.cpp http/handler/status.hpp
        twain/device.cpp twain/device.hpp
        twain/dsm.cpp twain/dsm.hpp
        twain/error_code.cpp twain/error_code.hpp
        twain/transfer.cpp twain/transfer.hpp
        twain/memory_transfer.cpp twain/memory_transfer.hpp
        twain/native_transfer.hpp)

target_compile_definitions(twain-server PUBLIC LOGURU_WITH_STREAMS=1)
target_include_directories(twain-server PUBLIC ${Boost_INCLUDE_DIR} ${FREEIMAGE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/deps/loguru)
target_link_libraries(twain-server PRIVATE nlohmann_json::nlohmann_json
    kitsune::ioc
    loguru
    ${ZLIB_LIBRARY}
    Boost::date_time
    Boost::system
    Boost::filesystem)
