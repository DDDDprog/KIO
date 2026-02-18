/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: Zeo-3.0-only
*/

#include "axeon/config.hpp"
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <regex>
#include <filesystem>
#include <iostream>

namespace kio {

static void parseAliasList(const char *env, std::unordered_map<std::string, std::string> &out) {
    if (!env) return;
    std::stringstream ss(env);
    std::string item;
    while (std::getline(ss, item, ',')) {
        auto pos = item.find('=');
        if (pos == std::string::npos) continue;
        std::string key = item.substr(0, pos);
        std::string val = item.substr(pos + 1);
        if (!key.empty() && !val.empty()) out[key] = val;
    }
}

Config Config::fromEnv() {
    Config cfg;
    parseAliasList(std::getenv("AXEON_ALIASES"), cfg.aliases);
    // Load from JSON file if present: env AXEON_CONFIG or ~/.axeon/config.json
    try {
        std::filesystem::path path;
        if (const char *p = std::getenv("AXEON_CONFIG")) {
            path = p;
        } else if (const char *home = std::getenv("HOME")) {
            path = std::filesystem::path(home) / ".axeon" / "config.json";
        }
        if (!path.empty() && std::filesystem::exists(path)) {
            std::ifstream ifs(path);
            if (!ifs.is_open()) {
                std::cerr << "Error: Could not open config file: " << path << std::endl;
                return cfg;
            }
            std::stringstream buffer; buffer << ifs.rdbuf();
            std::string json = buffer.str();
            
            // Robust regex for: {"aliases": {"print":"echo", "let":"var"}}
            // Handles whitespace around colons and braces better
            std::regex pairRe(R"(\"aliases\"\s*:\s*\{([^}]*)\})");
            std::smatch m;
            if (std::regex_search(json, m, pairRe)) {
                std::string body = m[1].str();
                std::regex kvRe(R"(\"([^\"]+)\"\s*:\s*\"([^\"]+)\")");
                auto it = std::sregex_iterator(body.begin(), body.end(), kvRe);
                auto end = std::sregex_iterator();
                for (; it != end; ++it) {
                    std::string key = (*it)[1].str();
                    std::string val = (*it)[2].str();
                    if (!key.empty() && !val.empty()) {
                        cfg.aliases[key] = val;
                    }
                }
            } else if (!json.empty()) {
                std::cerr << "Warning: 'aliases' section not found or malformed in " << path << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Config parsing error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error during config parsing" << std::endl;
    }
    return cfg;
}

} // namespace kio
