#pragma once

#include <string>
#include <vector>

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

struct TextEdit {
    Range range;
    std::string newText;
    
    TextEdit(Range r = Range(), const std::string& text = "") 
        : range(r), newText(text) {}
};

struct FormattingOptions {
    int tabSize = 4;
    bool insertSpaces = true;
    bool trimTrailingWhitespace = true;
    bool insertFinalNewline = true;
    bool trimFinalNewlines = true;
};

class DocumentFormatter {
public:
    DocumentFormatter();
    ~DocumentFormatter();
    
    std::vector<TextEdit> formatDocument(const std::string& content, 
                                       const FormattingOptions& options);
    
    std::vector<TextEdit> formatRange(const std::string& content, 
                                    const Range& range,
                                    const FormattingOptions& options);

private:
    std::string formatLine(const std::string& line, int indentLevel, 
                          const FormattingOptions& options);
    int calculateIndentLevel(const std::string& line);
    std::string createIndent(int level, const FormattingOptions& options);
};

} // namespace lsp
} // namespace kio
