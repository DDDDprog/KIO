cmake_minimum_required(VERSION 3.16)

project(kio VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

option(KIO_ENABLE_LTO "Enable Link Time Optimization" ON)
option(KIO_STRIP_SYMBOLS "Strip symbols on release" ON)
option(KIO_ENABLE_NATIVE_ARCH "Enable native architecture optimizations" ON)
option(KIO_ENABLE_FAST_MATH "Enable fast math optimizations" ON)
option(KIO_ENABLE_JIT "Enable Just-In-Time compilation" ON)
option(KIO_ENABLE_PARALLEL "Enable parallel execution" ON)
option(KIO_BUILD_STATIC "Build static executable" OFF)
option(KIO_BUILD_LSP "Build Language Server Protocol server" ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(KIO_PLATFORM "WINDOWS")
    add_compile_definitions(KIO_PLATFORM_WINDOWS=1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(KIO_PLATFORM "MACOS")
    add_compile_definitions(KIO_PLATFORM_MACOS=1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(KIO_PLATFORM "LINUX")
    add_compile_definitions(KIO_PLATFORM_LINUX=1)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-flto>
        $<$<CONFIG:Release>:-fomit-frame-pointer>
        $<$<CONFIG:Release>:-funroll-loops>
        $<$<CONFIG:Release>:-finline-functions>
        $<$<CONFIG:RelWithDebInfo>:-O2>
    )
    
    if(KIO_ENABLE_NATIVE_ARCH)
        add_compile_options($<$<CONFIG:Release>:-march=native>)
    endif()
    
    if(KIO_ENABLE_FAST_MATH)
        add_compile_options($<$<CONFIG:Release>:-ffast-math>)
    endif()
    
    add_link_options($<$<CONFIG:Release>:-s>)
    
    if (KIO_ENABLE_LTO)
        include(CheckIPOSupported)
        check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT LTO_ERROR)
        if (LTO_SUPPORTED)
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
        endif()
    endif()
elseif (MSVC)
    add_compile_options(
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/GL>
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/Ot>
        $<$<CONFIG:Release>:/favor:INTEL64>
    )
    if (KIO_ENABLE_LTO)
        add_link_options($<$<CONFIG:Release>:/LTCG>)
    endif()
endif()

find_package(Threads REQUIRED)

if(KIO_ENABLE_JIT)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(LLVM QUIET llvm)
    endif()
endif()

if(PkgConfig_FOUND)
    pkg_check_modules(JSON QUIET nlohmann_json)
endif()

enable_testing()

add_library(kio_core
    src/lexer.cpp
    src/parser.cpp
    src/interpreter.cpp
    src/config.cpp
)

# Optionally include LSP sources when enabled
if (KIO_BUILD_LSP)
    target_sources(kio_core PRIVATE
        src/lsp/lsp_server.cpp
        src/lsp/document_manager.cpp
        src/lsp/diagnostics.cpp
        src/lsp/completion.cpp
        src/lsp/hover.cpp
        src/lsp/goto_definition.cpp
        src/lsp/formatting.cpp
        src/lsp/semantic_tokens.cpp
    )
endif()

target_include_directories(kio_core
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(kio_core PUBLIC Threads::Threads)

if(WIN32)
    target_link_libraries(kio_core PRIVATE ws2_32 winmm)
elseif(APPLE)
    target_link_libraries(kio_core PRIVATE "-framework CoreFoundation")
elseif(UNIX)
    target_link_libraries(kio_core PRIVATE dl rt)
endif()

if(KIO_ENABLE_JIT AND LLVM_FOUND)
    target_link_libraries(kio_core PRIVATE ${LLVM_LIBRARIES})
    target_include_directories(kio_core PRIVATE ${LLVM_INCLUDE_DIRS})
    target_compile_definitions(kio_core PRIVATE KIO_JIT_ENABLED=1)
endif()

if(KIO_BUILD_LSP)
    add_executable(kio-lsp
        src/lsp/main.cpp
    )
    
    target_link_libraries(kio-lsp PRIVATE kio_core)
    
    if(JSON_FOUND)
        target_link_libraries(kio-lsp PRIVATE ${JSON_LIBRARIES})
        target_include_directories(kio-lsp PRIVATE ${JSON_INCLUDE_DIRS})
        target_compile_definitions(kio-lsp PRIVATE KIO_JSON_ENABLED=1)
    endif()
    
    install(TARGETS kio-lsp RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

add_executable(kio
    src/main.cpp
)

target_link_libraries(kio PRIVATE kio_core)

if(KIO_BUILD_STATIC)
    if(WIN32)
        target_link_options(kio PRIVATE -static)
    else()
        target_link_options(kio PRIVATE -static-libgcc -static-libstdc++)
    endif()
endif()

target_compile_definitions(kio PUBLIC KIO_VERSION="${PROJECT_VERSION}")

install(TARGETS kio RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CTest)
add_test(NAME kio_hello COMMAND kio ${PROJECT_SOURCE_DIR}/examples/hello.kio)
set_tests_properties(kio_hello PROPERTIES PASS_REGULAR_EXPRESSION "^42\nkio\n$")

# Only enable extended tests when the corresponding features are enabled
if (KIO_ENABLE_PARALLEL)
    add_test(NAME kio_parallel COMMAND kio ${PROJECT_SOURCE_DIR}/examples/parallel_test.kio)
endif()

if (KIO_ENABLE_JIT)
    add_test(NAME kio_performance COMMAND kio ${PROJECT_SOURCE_DIR}/examples/performance_test.kio)
endif()

if (KIO_BUILD_LSP)
    add_test(NAME kio_modules COMMAND kio ${PROJECT_SOURCE_DIR}/examples/module_test.kio)
endif()

# Types demo remains simple and supported
add_test(NAME kio_types COMMAND kio ${PROJECT_SOURCE_DIR}/examples/type_test.kio)

set(CPACK_PACKAGE_NAME "kio")
set(CPACK_PACKAGE_VENDOR "kio-project")
set(CPACK_PACKAGE_CONTACT "kio@local")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "KIO: Ultra-fast, modern programming language")
set(CPACK_PACKAGE_VERSION_MAJOR 2)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)

if (WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS;WIX")
    set(CPACK_NSIS_DISPLAY_NAME "KIO Programming Language")
    set(CPACK_NSIS_PACKAGE_NAME "kio")
    set(CPACK_NSIS_CONTACT "kio@local")
    set(CPACK_WIX_UPGRADE_GUID "12345678-1234-1234-1234-123456789012")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ;DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "KIO")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ;AppImage")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "kio")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Languages")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
endif()

include(CPack)
