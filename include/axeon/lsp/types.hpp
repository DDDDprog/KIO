/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace kio::lsp {

struct Position {
    int line;
    int character;
    
    Position(int l = 0, int c = 0) : line(l), character(c) {}
};

struct Range {
    Position start;
    Position end;
};

struct Location {
    std::string uri;
    Range range;
};

struct TextEdit {
    Range range;
    std::string newText;
};

enum class CompletionItemKind {
    Text = 1,
    Method = 2,
    Function = 3,
    Constructor = 4,
    Field = 5,
    Variable = 6,
    Class = 7,
    Interface = 8,
    Module = 9,
    Property = 10,
    Unit = 11,
    Value = 12,
    Enum = 13,
    Keyword = 14,
    Snippet = 15,
    Color = 16,
    File = 17,
    Reference = 18
};

enum class InsertTextFormat {
    PlainText = 1,
    Snippet = 2
};

struct CompletionItem {
    std::string label;
    std::string detail;
    std::string documentation;
    CompletionItemKind kind;
    std::string insert_text;
    InsertTextFormat insert_text_format {InsertTextFormat::PlainText};
};

enum class CompletionType {
    General,
    Variable,
    Function,
    Module
};

struct CompletionContext {
    std::string current_line;
    std::string prefix;
    CompletionType type;
};

enum class DiagnosticSeverity {
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4
};

struct Diagnostic {
    Range range;
    DiagnosticSeverity severity;
    std::string message;
    std::string source;
};

struct Hover {
    std::string contents;
    std::optional<Range> range;
};

struct WordRange {
    size_t start;
    size_t length;
};

enum class SemanticTokenType {
    Namespace, Type, Class, Enum, Interface, Struct, TypeParameter, Parameter,
    Variable, Property, EnumMember, Event, Function, Method, Macro, Keyword,
    Modifier, Comment, String, Number, Regexp, Operator
};

enum class SemanticTokenModifier {
    Declaration, Definition, Readonly, Static, Deprecated, Abstract, Async,
    Modification, Documentation, DefaultLibrary
};

struct SemanticToken {
    int line;
    int character;
    int length;
    uint32_t type;
    uint32_t modifiers; // bitmask
};

} // namespace kio::lsp
