/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/module_system.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace kio {

// Module Implementation
Module::Module(const std::string& name) : name_(name) {}
Module::~Module() = default;

void Module::addFunction(const std::string& name, void* func_ptr) {
    functions_[name] = func_ptr;
}

void Module::addVariable(const std::string& name, void* var_ptr) {
    variables_[name] = var_ptr;
}

void* Module::getFunction(const std::string& name) const {
    auto it = functions_.find(name);
    return (it != functions_.end()) ? it->second : nullptr;
}

void* Module::getVariable(const std::string& name) const {
    auto it = variables_.find(name);
    return (it != variables_.end()) ? it->second : nullptr;
}

std::vector<std::string> Module::getExportedFunctions() const {
    std::vector<std::string> names;
    for (const auto& pair : functions_) names.push_back(pair.first);
    return names;
}

std::vector<std::string> Module::getExportedVariables() const {
    std::vector<std::string> names;
    for (const auto& pair : variables_) names.push_back(pair.first);
    return names;
}

// ModuleSystem Implementation
ModuleSystem::ModuleSystem() {
    search_paths_.push_back(".");
    search_paths_.push_back("./modules");
}

ModuleSystem::~ModuleSystem() = default;

std::shared_ptr<Module> ModuleSystem::loadModule(const std::string& path_str) {
    std::filesystem::path p(path_str);
    std::string name = p.stem().string();
    
    if (modules_.find(name) != modules_.end()) {
        return modules_[name];
    }
    
    auto module = std::make_shared<Module>(name);
    modules_[name] = module;
    return module;
}

std::shared_ptr<Module> ModuleSystem::getModule(const std::string& name) const {
    auto it = modules_.find(name);
    return (it != modules_.end()) ? it->second : nullptr;
}

void ModuleSystem::registerModule(const std::string& name, std::shared_ptr<Module> module) {
    modules_[name] = module;
}

void ModuleSystem::unloadModule(const std::string& name) {
    modules_.erase(name);
}

std::vector<std::string> ModuleSystem::getLoadedModules() const {
    std::vector<std::string> names;
    for (const auto& pair : modules_) names.push_back(pair.first);
    return names;
}

bool ModuleSystem::isModuleLoaded(const std::string& name) const {
    return modules_.find(name) != modules_.end();
}

std::string ModuleSystem::findModuleFile(const std::string& name) const {
    for (const auto& path : search_paths_) {
        std::filesystem::path p = std::filesystem::path(path) / (name + ".kio");
        if (std::filesystem::exists(p)) return p.string();
    }
    return "";
}

void ModuleSystem::loadBuiltinModules() {
    // Implement builtin modules if any
}

} // namespace kio
