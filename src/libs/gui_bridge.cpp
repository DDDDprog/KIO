/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/bytecode.hpp"
#include <iostream>
#include <string>

namespace kio {

// Native GUI bridge for KIO
Value native_gui_window(int argCount, Value* args) {
    if (argCount < 3) return NIL_VAL;
    std::string title = args[0].toString();
    int width = (int)args[1].toNumber();
    int height = (int)args[2].toNumber();
    
    std::cout << "[GUI] Creating window: " << title << " (" << width << "x" << height << ")\n";
    return TRUE_VAL;
}

Value native_gui_button(int argCount, Value* args) {
    if (argCount < 1) return NIL_VAL;
    std::string text = args[0].toString();
    std::cout << "[GUI] Rendering button: " << text << "\n";
    return NIL_VAL;
}

} // namespace kio
