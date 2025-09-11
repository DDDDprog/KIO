#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace kio {

class Module {
public:
    Module(const std::string& name);
    ~Module();
    
    void addFunction(const std::string& name, void* func_ptr);
    void addVariable(const std::string& name, void* var_ptr);
    void* getFunction(const std::string& name) const;
    void* getVariable(const std::string& name) const;
    
    const std::string& getName() const { return name_; }
    std::vector<std::string> getExportedFunctions() const;
    std::vector<std::string> getExportedVariables() const;

private:
    std::string name_;
    std::unordered_map<std::string, void*> functions_;
    std::unordered_map<std::string, void*> variables_;
};

class ModuleSystem {
public:
    ModuleSystem();
    ~ModuleSystem();
    
    std::shared_ptr<Module> loadModule(const std::string& path);
    std::shared_ptr<Module> getModule(const std::string& name) const;
    void registerModule(const std::string& name, std::shared_ptr<Module> module);
    void unloadModule(const std::string& name);
    
    std::vector<std::string> getLoadedModules() const;
    bool isModuleLoaded(const std::string& name) const;

private:
    std::unordered_map<std::string, std::shared_ptr<Module>> modules_;
    std::vector<std::string> search_paths_;
    
    std::string findModuleFile(const std::string& name) const;
    void loadBuiltinModules();
};

} // namespace kio
