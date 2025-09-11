#include "kio/config.hpp"
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <regex>
#include <filesystem>

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
    parseAliasList(std::getenv("KIO_ALIASES"), cfg.aliases);
    // Load from JSON file if present: env KIO_CONFIG or ~/.kio/config.json
    try {
        std::filesystem::path path;
        if (const char *p = std::getenv("KIO_CONFIG")) {
            path = p;
        } else if (const char *home = std::getenv("HOME")) {
            path = std::filesystem::path(home) / ".kio" / "config.json";
        }
        if (!path.empty() && std::filesystem::exists(path)) {
            std::ifstream ifs(path);
            std::stringstream buffer; buffer << ifs.rdbuf();
            std::string json = buffer.str();
            // Extremely small JSON extractor for: {"aliases": {"print":"echo", "let":"var"}}
            std::regex pairRe("\\\"aliases\\\"\\s*:\\s*\\{([^}]*)\\}");
            std::smatch m;
            if (std::regex_search(json, m, pairRe)) {
                std::string body = m[1].str();
                std::regex kvRe("\\\"([^\\\"]+)\\\"\\s*:\\s*\\\"([^\\\"]+)\\\"");
                auto it = std::sregex_iterator(body.begin(), body.end(), kvRe);
                auto end = std::sregex_iterator();
                for (; it != end; ++it) {
                    std::string key = (*it)[1].str();
                    std::string val = (*it)[2].str();
                    if (!key.empty() && !val.empty()) cfg.aliases[key] = val;
                }
            }
        }
    } catch (...) {
        // Ignore config parsing errors silently for now
    }
    return cfg;
}

} // namespace kio
