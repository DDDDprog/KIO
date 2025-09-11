#pragma once

#include <string>
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

struct HoverInfo {
    std::string contents;
    std::optional<Range> range;
    
    HoverInfo(const std::string& c = "", std::optional<Range> r = std::nullopt) 
        : contents(c), range(r) {}
};

class HoverProvider {
public:
    HoverProvider();
    ~HoverProvider();
    
    std::optional<HoverInfo> getHover(const std::string& uri, const Position& position);
    
private:
    std::optional<HoverInfo> getVariableHover(const std::string& name, const Position& pos);
    std::optional<HoverInfo> getFunctionHover(const std::string& name, const Position& pos);
    std::optional<HoverInfo> getKeywordHover(const std::string& keyword, const Position& pos);
};

} // namespace lsp
} // namespace kio
