#include "kio/lsp/completion.hpp"
#include "kio/interpreter.hpp"
#include <sstream>
#include <vector>
#include <string>

namespace kio::lsp {

CompletionProvider::CompletionProvider() {
    initialize_builtin_completions();
}

CompletionProvider::~CompletionProvider() = default;

std::vector<CompletionItem> CompletionProvider::get_completions(
    const std::string& content, const Position& position) {
    
    std::vector<CompletionItem> completions;
    
    // Get context at cursor position
    auto context = analyze_context(content, position);
    
    // Add keyword completions
    add_keyword_completions(context, completions);
    
    // Add builtin function completions
    add_builtin_completions(context, completions);
    
    // Add variable completions
    add_variable_completions(context, completions);
    
    // Add module completions
    add_module_completions(context, completions);
    
    return completions;
}

CompletionContext CompletionProvider::analyze_context(const std::string& content, 
                                                     const Position& position) {
    CompletionContext context;
    
    // Find the current line
    std::istringstream stream(content);
    std::string line;
    int current_line = 0;
    
    while (std::getline(stream, line) && current_line <= position.line) {
        if (current_line == position.line) {
            context.current_line = line;
            context.prefix = line.substr(0, position.character);
            break;
        }
        current_line++;
    }
    
    // Determine completion type based on context
    if (context.prefix.find("import ") != std::string::npos) {
        context.type = CompletionType::Module;
    } else if (context.prefix.find("let ") != std::string::npos) {
        context.type = CompletionType::Variable;
    } else {
        context.type = CompletionType::General;
    }
    
    return context;
}

void CompletionProvider::add_keyword_completions(const CompletionContext& context,
                                                std::vector<CompletionItem>& completions) {
    static const std::vector<std::string> keywords = {
        "let", "const", "if", "else", "while", "for", "function", "return",
        "import", "export", "module", "class", "struct", "enum", "match",
        "async", "await", "try", "catch", "throw", "true", "false", "null"
    };
    
    for (const auto& keyword : keywords) {
        CompletionItem item;
        item.label = keyword;
        item.kind = CompletionItemKind::Keyword;
        item.detail = "KIO keyword";
        item.insert_text = keyword;
        completions.push_back(item);
    }
}

void CompletionProvider::add_builtin_completions(const CompletionContext& context,
                                                std::vector<CompletionItem>& completions) {
    for (const auto& builtin : builtin_completions_) {
        completions.push_back(builtin);
    }
}

void CompletionProvider::add_variable_completions(const CompletionContext& context,
                                                 std::vector<CompletionItem>& completions) {
    // This would integrate with the interpreter's symbol table
    // to provide context-aware variable completions
}

void CompletionProvider::add_module_completions(const CompletionContext& context,
                                               std::vector<CompletionItem>& completions) {
    static const std::vector<std::string> modules = {
        "std", "math", "string", "array", "map", "io", "fs", "net", "json"
    };
    
    if (context.type == CompletionType::Module) {
        for (const auto& module : modules) {
            CompletionItem item;
            item.label = module;
            item.kind = CompletionItemKind::Module;
            item.detail = "KIO standard module";
            item.insert_text = module;
            completions.push_back(item);
        }
    }
}

void CompletionProvider::initialize_builtin_completions() {
    // Built-in functions
    static const std::vector<std::pair<std::string, std::string>> builtins = {
        {"print", "print(value: any) -> void"},
        {"println", "println(value: any) -> void"},
        {"len", "len(collection: array|string|map) -> int"},
        {"push", "push(array: array, value: any) -> void"},
        {"pop", "pop(array: array) -> any"},
        {"map", "map(array: array, fn: function) -> array"},
        {"filter", "filter(array: array, fn: function) -> array"},
        {"reduce", "reduce(array: array, fn: function, initial: any) -> any"},
        {"range", "range(start: int, end: int, step?: int) -> array"},
        {"type", "type(value: any) -> string"},
        {"str", "str(value: any) -> string"},
        {"int", "int(value: any) -> int"},
        {"float", "float(value: any) -> float"},
        {"bool", "bool(value: any) -> bool"}
    };
    
    for (const auto& [name, signature] : builtins) {
        CompletionItem item;
        item.label = name;
        item.kind = CompletionItemKind::Function;
        item.detail = signature;
        item.insert_text = name + "($0)";
        item.insert_text_format = InsertTextFormat::Snippet;
        builtin_completions_.push_back(item);
    }
}

} // namespace kio::lsp
