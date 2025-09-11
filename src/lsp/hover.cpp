#include "kio/lsp/hover.hpp"
#include <sstream>

namespace kio::lsp {

HoverProvider::HoverProvider() = default;
HoverProvider::~HoverProvider() = default;

std::optional<Hover> HoverProvider::get_hover(const std::string& content, const Position& position) {
    // Find the word at the cursor position
    auto word_range = get_word_at_position(content, position);
    if (!word_range) return std::nullopt;
    
    std::string word = content.substr(word_range->start, word_range->length);
    
    // Get hover information for the word
    auto hover_info = get_hover_info(word);
    if (!hover_info) return std::nullopt;
    
    Hover hover;
    hover.contents = *hover_info;
    hover.range = position_range_to_lsp_range(content, *word_range);
    
    return hover;
}

std::optional<WordRange> HoverProvider::get_word_at_position(const std::string& content, 
                                                           const Position& position) {
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
    
    // Find word boundaries
    size_t start = offset;
    size_t end = offset;
    
    // Move start backward to find word start
    while (start > 0 && (std::isalnum(content[start - 1]) || content[start - 1] == '_')) {
        start--;
    }
    
    // Move end forward to find word end
    while (end < content.size() && (std::isalnum(content[end]) || content[end] == '_')) {
        end++;
    }
    
    if (start == end) return std::nullopt;
    
    return WordRange{start, end - start};
}

std::optional<std::string> HoverProvider::get_hover_info(const std::string& word) {
    // Built-in functions documentation
    static const std::unordered_map<std::string, std::string> builtin_docs = {
        {"print", "**print**(value: any) -> void\n\nPrints a value to stdout without newline."},
        {"println", "**println**(value: any) -> void\n\nPrints a value to stdout with newline."},
        {"len", "**len**(collection: array|string|map) -> int\n\nReturns the length of a collection."},
        {"push", "**push**(array: array, value: any) -> void\n\nAdds an element to the end of an array."},
        {"pop", "**pop**(array: array) -> any\n\nRemoves and returns the last element of an array."},
        {"map", "**map**(array: array, fn: function) -> array\n\nApplies a function to each element and returns a new array."},
        {"filter", "**filter**(array: array, fn: function) -> array\n\nFilters elements based on a predicate function."},
        {"reduce", "**reduce**(array: array, fn: function, initial: any) -> any\n\nReduces an array to a single value."},
        {"range", "**range**(start: int, end: int, step?: int) -> array\n\nGenerates a range of numbers."},
        {"type", "**type**(value: any) -> string\n\nReturns the type of a value as a string."},
        {"str", "**str**(value: any) -> string\n\nConverts a value to a string."},
        {"int", "**int**(value: any) -> int\n\nConverts a value to an integer."},
        {"float", "**float**(value: any) -> float\n\nConverts a value to a floating-point number."},
        {"bool", "**bool**(value: any) -> bool\n\nConverts a value to a boolean."}
    };
    
    // Keywords documentation
    static const std::unordered_map<std::string, std::string> keyword_docs = {
        {"let", "**let** - Variable declaration\n\nDeclares a mutable variable.\n\nExample: `let x = 42`"},
        {"const", "**const** - Constant declaration\n\nDeclares an immutable constant.\n\nExample: `const PI = 3.14159`"},
        {"if", "**if** - Conditional statement\n\nExecutes code based on a condition.\n\nExample: `if (x > 0) { ... }`"},
        {"else", "**else** - Alternative branch\n\nExecutes when if condition is false.\n\nExample: `if (x > 0) { ... } else { ... }`"},
        {"while", "**while** - Loop statement\n\nRepeats code while condition is true.\n\nExample: `while (x < 10) { ... }`"},
        {"for", "**for** - Iteration statement\n\nIterates over a collection or range.\n\nExample: `for (item in array) { ... }`"},
        {"function", "**function** - Function declaration\n\nDefines a reusable function.\n\nExample: `function add(a, b) { return a + b }`"},
        {"return", "**return** - Return statement\n\nReturns a value from a function.\n\nExample: `return result`"},
        {"import", "**import** - Module import\n\nImports functionality from a module.\n\nExample: `import math`"},
        {"export", "**export** - Module export\n\nExports functionality from a module.\n\nExample: `export function myFunc() { ... }`"}
    };
    
    if (auto it = builtin_docs.find(word); it != builtin_docs.end()) {
        return it->second;
    }
    
    if (auto it = keyword_docs.find(word); it != keyword_docs.end()) {
        return it->second;
    }
    
    return std::nullopt;
}

Range HoverProvider::position_range_to_lsp_range(const std::string& content, const WordRange& word_range) {
    // Convert byte offsets to line/character positions
    Position start_pos{0, 0};
    Position end_pos{0, 0};
    
    size_t current_offset = 0;
    int current_line = 0;
    int current_char = 0;
    
    for (size_t i = 0; i < content.size(); ++i) {
        if (current_offset == word_range.start) {
            start_pos = {current_line, current_char};
        }
        if (current_offset == word_range.start + word_range.length) {
            end_pos = {current_line, current_char};
            break;
        }
        
        if (content[i] == '\n') {
            current_line++;
            current_char = 0;
        } else {
            current_char++;
        }
        current_offset++;
    }
    
    return {start_pos, end_pos};
}

} // namespace kio::lsp
