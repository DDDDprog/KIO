/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include "axeon/bytecode.hpp"
#include <vector>
#include <string>

namespace kio {

Value native_server_init(int argCount, Value* args);
Value native_server_accept(int argCount, Value* args);
Value native_server_respond(int argCount, Value* args);

} // namespace kio
