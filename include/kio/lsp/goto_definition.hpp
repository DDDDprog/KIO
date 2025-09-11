/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <vector>
#include <optional>

namespace kio {
namespace lsp {

struct Position {
    int line;
    int character;
    
    Position(int l = 0, int c = 0) : line(l), character(c) {}
};

struct Range {
    Position start;
    Position end;
    
    Range(Position s = Position(), Position e = Position()) : start(s), end(e) {}
};

struct Location {
    std::string uri;
    Range range;
    
    Location(const std::string& u = "", Range r = Range()) : uri(u), range(r) {}
};

struct DefinitionInfo {
    std::string name;
    std::string type;
    Location location;
    
    DefinitionInfo(const std::string& n = "", const std::string& t = "", 
                   Location loc = Location()) 
        : name(n), type(t), location(loc) {}
};

class GotoDefinitionProvider {
public:
    GotoDefinitionProvider();
    ~GotoDefinitionProvider();
    
    std::vector<Location> getDefinition(const std::string& uri, const Position& position);
    std::vector<Location> getDeclaration(const std::string& uri, const Position& position);
    std::vector<Location> getReferences(const std::string& uri, const Position& position);

private:
    std::optional<DefinitionInfo> findSymbolAt(const std::string& content, 
                                              const Position& position);
    std::vector<Location> searchForDefinition(const std::string& symbolName);
    std::vector<Location> searchForReferences(const std::string& symbolName);
};

} // namespace lsp
} // namespace kio
