/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>

namespace kio {

enum class OS {
    LINUX,
    MACOS,
    WINDOWS,
    IOS,
    ANDROID,
    WASM
};

struct PlatformInfo {
    static OS current() {
#if defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        return OS::IOS;
    #else
        return OS::MACOS;
    #endif
#elif defined(__ANDROID__)
        return OS::ANDROID;
#elif defined(_WIN32)
        return OS::WINDOWS;
#elif defined(__wasm__)
        return OS::WASM;
#else
        return OS::LINUX;
#endif
    }

    static std::string get_name() {
        switch(current()) {
            case OS::MACOS: return "macOS (Silicon-Optimized)";
            case OS::IOS: return "iOS";
            case OS::ANDROID: return "Android";
            case OS::WASM: return "WebAssembly";
            case OS::WINDOWS: return "Windows";
            default: return "Linux";
        }
    }
    
    static std::string get_cpu_model();
    static long long get_total_memory();
    static long long get_root_disk_space();

    // Low-level hardware affinity (macOS Silicon optimizations)
    static void optimize_for_m1_m2() {
#if defined(__APPLE__) && defined(__aarch64__)
        // macOS Silicon specific performance affinity hints
#endif
    }
};

} // namespace kio
