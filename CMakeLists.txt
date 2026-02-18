# Copyright (c) 2025 Dipanjan Dhar
# SPDX-License-Identifier: Zeo-3.0-only

cmake_minimum_required(VERSION 3.16)

project(axeon VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

option(AXEON_ENABLE_LTO "Enable Link Time Optimization" ON)
option(AXEON_STRIP_SYMBOLS "Strip symbols on release" ON)
option(AXEON_ENABLE_NATIVE_ARCH "Enable native architecture optimizations" ON)
option(AXEON_ENABLE_FAST_MATH "Enable fast math optimizations" ON)
option(AXEON_ENABLE_JIT "Enable Just-In-Time compilation" ON)
option(AXEON_ENABLE_PARALLEL "Enable parallel execution" ON)
option(AXEON_BUILD_STATIC "Build static executable" OFF)
option(AXEON_BUILD_LSP "Build Language Server Protocol server" ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(AXEON_PLATFORM "WINDOWS")
    add_compile_definitions(AXEON_PLATFORM_WINDOWS=1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(AXEON_PLATFORM "MACOS")
    add_compile_definitions(AXEON_PLATFORM_MACOS=1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(AXEON_PLATFORM "LINUX")
    add_compile_definitions(AXEON_PLATFORM_LINUX=1)
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
    
    if(AXEON_ENABLE_NATIVE_ARCH)
        add_compile_options($<$<CONFIG:Release>:-march=native>)
    endif()
    
    if(AXEON_ENABLE_FAST_MATH)
        add_compile_options($<$<CONFIG:Release>:-ffast-math>)
    endif()
    
    add_link_options($<$<CONFIG:Release>:-s>)
    
    if (AXEON_ENABLE_LTO)
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
    if (AXEON_ENABLE_LTO)
        add_link_options($<$<CONFIG:Release>:/LTCG>)
    endif()
endif()

find_package(Threads REQUIRED)

if(AXEON_ENABLE_JIT)
    find_package(LLVM REQUIRED CONFIG)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
    llvm_map_components_to_libnames(llvm_libs core orcjit native executionengine mcjit bitreader support passes analysis ipo scalaropts)
endif()

enable_testing()

add_library(axeon_core
    src/language/lexer.cpp
    src/language/token_stream.cpp
    src/language/preprocessor.cpp
    src/language/parser.cpp
    src/language/parser_extensions.cpp
    src/language/error_recovery.cpp
    src/language/ast_visitor.cpp
    src/language/token.cpp
    src/runtime/interpreter.cpp
    src/runtime/value.cpp
    src/runtime/builtin_functions.cpp
    src/runtime/module_system.cpp
    src/compiler/vm.cpp
    src/compiler/compiler.cpp
    src/runtime/config.cpp
    src/runtime/platform.cpp
    src/compiler/optimizer.cpp
    src/compiler/memory_manager.cpp
    src/compiler/parallel_executor.cpp
    src/compiler/type_system.cpp
    src/runtime/type_impl.cpp
    src/libs/vector_ops.cpp
    src/libs/db_driver.cpp
    src/libs/gmp_bridge.cpp
    src/libs/aes.cpp
    src/libs/sha256.cpp
    src/libs/gpu_bridge.cpp
    src/libs/gui_bridge.cpp
    src/libs/http_server.cpp
    c_core/abi.cpp
)

if(AXEON_ENABLE_JIT)
    target_sources(axeon_core PRIVATE
        src/compiler/jit_compiler.cpp
        src/compiler/jit_engine.cpp
    )
    target_link_libraries(axeon_core PRIVATE ${llvm_libs})
    target_compile_definitions(axeon_core PRIVATE AXEON_JIT_ENABLED)
else()
    # When JIT is disabled, provide a lightweight stub so the VM still links.
    target_sources(axeon_core PRIVATE
        src/compiler/jit_engine_stub.cpp
    )
endif()

# Optionally include LSP sources when enabled
if (AXEON_BUILD_LSP)
    target_sources(axeon_core PRIVATE
        src/tools/lsp_server.cpp
        src/tools/document_manager.cpp
        src/tools/diagnostics.cpp
        src/tools/completion.cpp
        src/tools/hover.cpp
        src/tools/goto_definition.cpp
        src/tools/formatting.cpp
        src/tools/semantic_tokens.cpp
    )
endif()

target_include_directories(axeon_core
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

# Professional-Grade Build Polish: "Very Very Fast" compilation using PCH
target_precompile_headers(axeon_core PRIVATE
    <vector>
    <string>
    <iostream>
    <cmath>
    <memory>
    <map>
    <chrono>
)

target_link_libraries(axeon_core PUBLIC Threads::Threads)

if(WIN32)
    target_link_libraries(axeon_core PRIVATE ws2_32 winmm)
elseif(APPLE)
    target_link_libraries(axeon_core PRIVATE "-framework CoreFoundation")
elseif(UNIX)
    target_link_libraries(axeon_core PRIVATE dl rt gmp)
endif()

if(AXEON_ENABLE_JIT AND LLVM_FOUND)
    target_link_libraries(axeon_core PRIVATE ${LLVM_LIBRARIES})
    target_include_directories(axeon_core PRIVATE ${LLVM_INCLUDE_DIRS})
    target_compile_definitions(axeon_core PRIVATE AXEON_JIT_ENABLED=1 KIO_JIT_ENABLED=1)
endif()

if(AXEON_BUILD_LSP)
    add_executable(axeon-lsp
        src/tools/main.cpp
    )
    
    target_link_libraries(axeon-lsp PRIVATE axeon_core)
    
    if(JSON_FOUND)
        target_link_libraries(axeon-lsp PRIVATE ${JSON_LIBRARIES})
        target_include_directories(axeon-lsp PRIVATE ${JSON_INCLUDE_DIRS})
        target_compile_definitions(axeon-lsp PRIVATE AXEON_JSON_ENABLED=1)
    endif()
    
    install(TARGETS axeon-lsp RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

add_executable(axeon
    src/runtime/main.cpp
)

target_link_libraries(axeon PRIVATE axeon_core)

if(AXEON_BUILD_STATIC)
    if(WIN32)
        target_link_options(axeon PRIVATE -static)
    else()
        target_link_options(axeon PRIVATE -static-libgcc -static-libstdc++)
    endif()
endif()

target_compile_definitions(axeon PUBLIC AXEON_VERSION="${PROJECT_VERSION}")

install(TARGETS axeon RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CTest)
add_test(NAME axeon_hello COMMAND axeon ${PROJECT_SOURCE_DIR}/examples/hello.axe)
set_tests_properties(axeon_hello PROPERTIES PASS_REGULAR_EXPRESSION "42\n.*axeon")

# Core operator tests (always enabled)
add_test(NAME axeon_equality COMMAND axeon ${PROJECT_SOURCE_DIR}/examples/equality_test.axe)
set_tests_properties(axeon_equality PROPERTIES PASS_REGULAR_EXPRESSION "true\n.*false")

add_test(NAME axeon_comparisons COMMAND axeon ${PROJECT_SOURCE_DIR}/examples/comparisons_test.axe)
set_tests_properties(axeon_comparisons PROPERTIES PASS_REGULAR_EXPRESSION "true\n.*false\n.*true\n.*true")

add_test(NAME axeon_modulo COMMAND axeon ${PROJECT_SOURCE_DIR}/examples/modulo_test.axe)
set_tests_properties(axeon_modulo PROPERTIES PASS_REGULAR_EXPRESSION "1")

add_test(NAME axeon_arithmetic COMMAND axeon ${PROJECT_SOURCE_DIR}/examples/arithmetic_test.axe)
set_tests_properties(axeon_arithmetic PROPERTIES PASS_REGULAR_EXPRESSION "15\n.*5\n.*50\n.*true")

# Only enable extended tests when the corresponding features are enabled
if (AXEON_ENABLE_PARALLEL)
    add_test(NAME axeon_parallel COMMAND axeon ${PROJECT_SOURCE_DIR}/examples/parallel_test.axe)
endif()

if (AXEON_ENABLE_JIT)
    add_test(NAME axeon_performance COMMAND axeon ${PROJECT_SOURCE_DIR}/examples/performance_test.axe)
endif()

if (AXEON_BUILD_LSP)
    add_test(NAME axeon_modules COMMAND axeon ${PROJECT_SOURCE_DIR}/examples/module_test.axe)
endif()

# Types demo remains simple and supported
add_test(NAME axeon_types COMMAND axeon ${PROJECT_SOURCE_DIR}/examples/type_test.axe)

set(CPACK_PACKAGE_NAME "axeon")
set(CPACK_PACKAGE_VENDOR "axeon-project")
set(CPACK_PACKAGE_CONTACT "axeon@local")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Axeon: Ultra-fast, modern programming language")
set(CPACK_PACKAGE_VERSION_MAJOR 2)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)

if (WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS;WIX")
    set(CPACK_NSIS_DISPLAY_NAME "Axeon Programming Language")
    set(CPACK_NSIS_PACKAGE_NAME "axeon")
    set(CPACK_NSIS_CONTACT "axeon@local")
    set(CPACK_WIX_UPGRADE_GUID "12345678-1234-1234-1234-123456789012")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ;DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "Axeon")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ;AppImage")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "axeon")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Languages")
    set(CPACK_RPM_PACKAGE_LICENSE "Zeo")
endif()

include(CPack)
