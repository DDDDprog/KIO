#include "kio/module_system.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace kio {

ModuleSystem::ModuleSystem() = default;
ModuleSystem::~ModuleSystem() = default;

bool ModuleSystem::loadModule(const std::string& moduleName, const std::string& path) {
    if (loadedModules.find(moduleName) != loadedModules.end()) {
        return true; // Already loaded
    }
    
    std::filesystem::path modulePath(path);
    if (!std::filesystem::exists(modulePath)) {
        std::cerr << "Module not found: " << path << std::endl;
        return false;
    }
    
    std::ifstream file(modulePath);
    if (!file.is_open()) {
        std::cerr << "Cannot open module: " << path << std::endl;
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    Module module;
    module.name = moduleName;
    module.path = path;
    module.content = content;
    module.loaded = true;
    
    loadedModules[moduleName] = std::move(module);
    std::cout << "Loaded module: " << moduleName << std::endl;
    return true;
}

bool ModuleSystem::unloadModule(const std::string& moduleName) {
    auto it = loadedModules.find(moduleName);
    if (it != loadedModules.end()) {
        loadedModules.erase(it);
        std::cout << "Unloaded module: " << moduleName << std::endl;
        return true;
    }
    return false;
}

Module* ModuleSystem::getModule(const std::string& moduleName) {
    auto it = loadedModules.find(moduleName);
    return (it != loadedModules.end()) ? &it->second : nullptr;
}

std::vector<std::string> ModuleSystem::getLoadedModules() const {
    std::vector<std::string> modules;
    for (const auto& pair : loadedModules) {
        modules.push_back(pair.first);
    }
    return modules;
}

bool ModuleSystem::importSymbol(const std::string& moduleName, const std::string& symbolName) {
    Module* module = getModule(moduleName);
    if (!module) {
        std::cerr << "Module not loaded: " << moduleName << std::endl;
        return false;
    }
    
    // Add symbol to current scope
    importedSymbols[symbolName] = moduleName;
    std::cout << "Imported symbol: " << symbolName << " from " << moduleName << std::endl;
    return true;
}

} // namespace kio
