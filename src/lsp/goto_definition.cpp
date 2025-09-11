#include "kio/lsp/goto_definition.hpp"

namespace kio::lsp {

GotoDefinitionProvider::GotoDefinitionProvider() = default;
GotoDefinitionProvider::~GotoDefinitionProvider() = default;

std::optional<Location> GotoDefinitionProvider::get_definition(
    const std::string& content, const Position& position) {
    
    // Find the symbol at the cursor position
    auto symbol = get_symbol_at_position(content, position);
    if (!symbol) return std::nullopt;
    
    // Find the definition of the symbol
    return find_symbol_definition(*symbol, content);
}

std::optional<std::string> GotoDefinitionProvider::get_symbol_at_position(
    const std::string& content, const Position& position) {
    
    // Convert position to offset
    size_t offset = 0;
    int current_line = 0;
    int current_char = 0;
    
    for (size_t i = 0; i < content.size(); ++i) {
        if (current_line == position.line && current_char == position.character) {
            offset = i;
            break;
        }
        
        if (content[i] == '\n') {
            current_line++;
            current_char = 0;
        } else {
            current_char++;
        }
    }
    
    // Find symbol boundaries
    size_t start = offset;
    size_t end = offset;
    
    // Move start backward
    while (start > 0 && (std::isalnum(content[start - 1]) || content[start - 1] == '_')) {
        start--;
    }
    
    // Move end forward
    while (end < content.size() && (std::isalnum(content[end]) || content[end] == '_')) {
        end++;
    }
    
    if (start == end) return std::nullopt;
    
    return content.substr(start, end - start);
}

std::optional<Location> GotoDefinitionProvider::find_symbol_definition(
    const std::string& symbol, const std::string& content) {
    
    // Look for function definitions
    std::string function_pattern = "function " + symbol + "(";
    size_t pos = content.find(function_pattern);
    if (pos != std::string::npos) {
        auto position = offset_to_position(content, pos);
        return Location{"", {position, position}};
    }
    
    // Look for variable declarations
    std::string let_pattern = "let " + symbol + " =";
    pos = content.find(let_pattern);
    if (pos != std::string::npos) {
        auto position = offset_to_position(content, pos);
        return Location{"", {position, position}};
    }
    
    std::string const_pattern = "const " + symbol + " =";
    pos = content.find(const_pattern);
    if (pos != std::string::npos) {
        auto position = offset_to_position(content, pos);
        return Location{"", {position, position}};
    }
    
    return std::nullopt;
}

Position GotoDefinitionProvider::offset_to_position(const std::string& content, size_t offset) {
    Position pos{0, 0};
    for (size_t i = 0; i < offset && i < content.size(); ++i) {
        if (content[i] == '\n') {
            pos.line++;
            pos.character = 0;
        } else {
            pos.character++;
        }
    }
    return pos;
}

} // namespace kio::lsp
