/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include <emscripten.h>
#include <iostream>
#include "kio/vm.hpp"

namespace kio {

extern "C" {

EMSCRIPTEN_KEEPALIVE
const char* run_kio_wasm(const char* source) {
    // WASM high-speed entry point
    std::cout << "KIO WASM Engine Started...\n";
    // Initialize VM and run...
    return "OK";
}

}

} // namespace kio
