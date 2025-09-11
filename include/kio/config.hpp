#pragma once

#include <string>
#include <unordered_map>

namespace kio {

struct Config {
    // Map of logical command -> keyword used in code (e.g., print -> echo)
    std::unordered_map<std::string, std::string> aliases;
    static Config fromEnv();
};

} // namespace kio
