import * as vscode from 'vscode';

// Complete list of Axeon language keywords
const KEYWORDS = [
    // Control flow
    'if', 'elif', 'else', 'then', 'end',
    'for', 'while', 'do', 'break', 'continue',
    'return', 'match', 'case', 'switch',
    // Import/Export
    'import', 'export', 'module', 'as', 'from', 'use',
    // Functions
    'fn', 'async', 'await', 'yield', 'throw', 'try',
    // Types
    'class', 'struct', 'enum', 'interface', 'trait', 'impl',
    'pub', 'priv', 'static', 'const', 'let', 'var', 'mut', 'type',
    'extends', 'implements', 'super',
    // Exception handling
    'catch', 'panic', 'assert', 'finally',
    // Operators
    'in', 'is', 'not', 'and', 'or', 'as', 'where', 'of',
    // Values
    'true', 'false', 'nil', 'self', 'this', 'new', 'delete',
    // Other
    'sizeof', 'typeof', 'unsafe', 'extern'
];

// Complete list of built-in functions
const BUILTIN_FUNCTIONS = [
    // I/O
    'print', 'println', 'input', 'format',
    // Type conversion
    'type', 'to_string', 'to_int', 'to_float', 'to_bool', 'to_array',
    // Collection
    'len', 'range', 'map', 'filter', 'reduce', 'zip', 'enumerate',
    'join', 'split', 'trim', 'upper', 'lower', 'replace', 'contains',
    'startswith', 'endswith', 'index_of', 'char_at', 'substring',
    'to_upper', 'to_lower', 'reverse', 'sort', 'unique',
    'flatten', 'flat_map', 'for_each', 'any', 'all', 'none',
    'sum', 'avg', 'product', 'count', 'first', 'last',
    'take', 'drop', 'skip', 'take_while', 'drop_while',
    'partition', 'group_by',
    // Iterators
    'into_iter', 'iter', 'iter_mut', 'collect',
    // Math
    'abs', 'min', 'max', 'pow', 'sqrt', 'floor', 'ceil', 'round',
    'sin', 'cos', 'tan', 'asin', 'acos', 'atan', 'atan2',
    'log', 'log10', 'exp', 'sign',
    // Random
    'rand', 'rand_int', 'rand_float', 'rand_bool', 'seed',
    // Time
    'time', 'sleep', 'timestamp', 'datetime', 'format_time',
    // System
    'exit', 'panic', 'assert', 'debug', 'trace', 'env', 'args',
    // File
    'open', 'read', 'read_line', 'write', 'write_line', 'close',
    'exists', 'mkdir', 'remove', 'rename', 'copy',
    // HTTP
    'http_get', 'http_post', 'http_put', 'http_delete', 'http_request',
    // JSON
    'json_parse', 'json_stringify', 'json_to_map', 'json_to_array',
    // Crypto
    'sha256', 'sha512', 'md5', 'aes_encrypt', 'aes_decrypt',
    'base64_encode', 'base64_decode', 'hash',
    // Database
    'db_connect', 'db_query', 'db_execute', 'db_close',
    // Network
    'tcp_connect', 'tcp_listen', 'udp_send', 'udp_receive',
    // Memory
    'clone', 'copy', 'ref', 'move', 'drop', 'size_of', 'align_of',
    // String
    'str', 'string', 'to_str', 'from_str',
    // Option
    'some', 'none', 'is_some', 'is_none', 'unwrap', 'unwrap_or',
    'map_or', 'ok', 'err', 'is_ok', 'is_err',
    // Vector
    'vec', 'vector', 'with_capacity', 'push', 'pop', 'insert', 'remove_at'
];

// Storage types
const TYPES = [
    'i8', 'i16', 'i32', 'i64', 'i128',
    'u8', 'u16', 'u32', 'u64', 'u128',
    'f32', 'f64',
    'bool', 'char', 'str', 'string',
    'int', 'float', 'void', 'any', 'type', 'Type', 'Any',
    'usize', 'isize'
];

// Create completion items
const KEYWORD_COMPLETIONS: vscode.CompletionItem[] = KEYWORDS.map(keyword => {
    const item = new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
    item.detail = `Axeon keyword: ${keyword}`;
    item.documentation = new vscode.MarkdownString(`**${keyword}** - Axeon language keyword`);
    return item;
});

const FUNCTION_COMPLETIONS: vscode.CompletionItem[] = BUILTIN_FUNCTIONS.map(func => {
    const item = new vscode.CompletionItem(func, vscode.CompletionItemKind.Function);
    item.detail = `Axeon built-in function: ${func}()`;
    item.insertText = new vscode.SnippetString(`${func}(\$0)`);
    item.documentation = new vscode.MarkdownString(`**${func}()** - Axeon built-in function`);
    return item;
});

const TYPE_COMPLETIONS: vscode.CompletionItem[] = TYPES.map(t => {
    const item = new vscode.CompletionItem(t, vscode.CompletionItemKind.TypeParameter);
    item.detail = `Axeon type: ${t}`;
    item.documentation = new vscode.MarkdownString(`**${t}** - Axeon primitive type`);
    return item;
});

export function activateCompletionProvider(context: vscode.ExtensionContext) {
    const completionProvider = vscode.languages.registerCompletionItemProvider(
        { language: 'axeon', scheme: 'file' },
        {
            provideCompletionItems(document: vscode.TextDocument, position: vscode.Position) {
                const completions: vscode.CompletionList = new vscode.CompletionList([
                    ...KEYWORD_COMPLETIONS,
                    ...FUNCTION_COMPLETIONS,
                    ...TYPE_COMPLETIONS,
                    ...getSnippetCompletions()
                ]);
                
                return completions;
            }
        },
        '.', // Trigger on dot
        '(', // Trigger on parenthesis
        ' '  // Trigger on space for keywords
    );

    context.subscriptions.push(completionProvider);
}

function getSnippetCompletions(): vscode.CompletionItem[] {
    return [
        // Function snippets
        {
            label: 'fn',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Function definition',
            insertText: new vscode.SnippetString('fn ${1:function_name}(${2:args}) {\n\t$0\n}'),
            documentation: 'Define a new function'
        },
        {
            label: 'fn-arrow',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Arrow function',
            insertText: new vscode.SnippetString('fn ${1:args} => ${2:expression}'),
            documentation: 'Define an arrow function'
        },
        {
            label: 'async-fn',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Async function',
            insertText: new vscode.SnippetString('async fn ${1:function_name}(${2:args}) {\n\t$0\n}'),
            documentation: 'Define an async function'
        },
        
        // Loop snippets
        {
            label: 'for',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'For loop',
            insertText: new vscode.SnippetString('for ${1:i} in ${2:range(10)} {\n\t$0\n}'),
            documentation: 'For loop iteration'
        },
        {
            label: 'for-i',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'For loop with index',
            insertText: new vscode.SnippetString('for ${1:i}, ${2:item} in enumerate(${3:collection}) {\n\t$0\n}'),
            documentation: 'For loop with index'
        },
        {
            label: 'while',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'While loop',
            insertText: new vscode.SnippetString('while ${1:condition} {\n\t$0\n}'),
            documentation: 'While loop'
        },
        
        // Conditional snippets
        {
            label: 'if',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'If statement',
            insertText: new vscode.SnippetString('if ${1:condition} {\n\t$0\n}'),
            documentation: 'If conditional statement'
        },
        {
            label: 'if-else',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'If-else statement',
            insertText: new vscode.SnippetString('if ${1:condition} {\n\t$2\n} else {\n\t$0\n}'),
            documentation: 'If-else conditional statement'
        },
        {
            label: 'if-elif',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'If-elif-else statement',
            insertText: new vscode.SnippetString('if ${1:condition1} {\n\t$2\n} elif ${3:condition2} {\n\t$4\n} else {\n\t$0\n}'),
            documentation: 'If-elif-else conditional statement'
        },
        
        // Match snippet
        {
            label: 'match',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Match expression',
            insertText: new vscode.SnippetString('match ${1:expr} {\n\t${2:pattern} => ${3:value},\n\t_ => ${4:default},\n}'),
            documentation: 'Match expression (pattern matching)'
        },
        
        // Type snippets
        {
            label: 'class',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Class definition',
            insertText: new vscode.SnippetString('class ${1:ClassName} {\n\tfn __init__(self${2:, args}) {\n\t\t$0\n\t}\n}'),
            documentation: 'Define a new class'
        },
        {
            label: 'struct',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Struct definition',
            insertText: new vscode.SnippetString('struct ${1:StructName} {\n\t${2:field1}: ${3:type1},\n\t${4:field2}: ${5:type2},\n}'),
            documentation: 'Define a new struct'
        },
        {
            label: 'enum',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Enum definition',
            insertText: new vscode.SnippetString('enum ${1:EnumName} {\n\t${2:Variant1},\n\t${3:Variant2}(${4:data}),\n}'),
            documentation: 'Define a new enum'
        },
        {
            label: 'interface',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Interface definition',
            insertText: new vscode.SnippetString('interface ${1:InterfaceName} {\n\tfn ${2:method}(${3:self});\n}'),
            documentation: 'Define a new interface'
        },
        {
            label: 'impl',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Impl block',
            insertText: new vscode.SnippetString('impl ${1:TypeName} {\n\t$0\n}'),
            documentation: 'Implementation block'
        },
        
        // Module snippets
        {
            label: 'module',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Module definition',
            insertText: new vscode.SnippetString('module ${1:module_name} {\n\t$0\n}'),
            documentation: 'Define a new module'
        },
        {
            label: 'import',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Import statement',
            insertText: new vscode.SnippetString('import ${1:module_name}'),
            documentation: 'Import a module'
        },
        {
            label: 'import-as',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Import with alias',
            insertText: new vscode.SnippetString('import ${1:module_name} as ${2:alias}'),
            documentation: 'Import a module with alias'
        },
        
        // Exception handling
        {
            label: 'try-catch',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Try-catch block',
            insertText: new vscode.SnippetString('try {\n\t$1\n} catch ${2:error} {\n\t$0\n}'),
            documentation: 'Try-catch exception handling'
        },
        {
            label: 'try-catch-finally',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Try-catch-finally block',
            insertText: new vscode.SnippetString('try {\n\t$1\n} catch ${2:error} {\n\t$3\n} finally {\n\t$0\n}'),
            documentation: 'Try-catch-finally exception handling'
        },
        
        // Test snippets
        {
            label: 'test',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Test function',
            insertText: new vscode.SnippetString('#[test]\nfn ${1:test_name}() {\n\t$0\n}'),
            documentation: 'Define a test function'
        },
        {
            label: 'bench',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Benchmark function',
            insertText: new vscode.SnippetString('#[bench]\nfn ${1:bench_name}(b) {\n\t$0\n}'),
            documentation: 'Define a benchmark function'
        },
        
        // Decorators
        {
            label: '@test',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Test decorator',
            insertText: new vscode.SnippetString('#[test]'),
            documentation: 'Test decorator'
        },
        {
            label: '@bench',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Benchmark decorator',
            insertText: new vscode.SnippetString('#[bench]'),
            documentation: 'Benchmark decorator'
        },
        {
            label: '@inline',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Inline decorator',
            insertText: new vscode.SnippetString('#[inline]'),
            documentation: 'Inline decorator'
        },
        
        // Common patterns
        {
            label: 'main',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Main function',
            insertText: new vscode.SnippetString('fn main() {\n\t$0\n}'),
            documentation: 'Define main function'
        },
        {
            label: 'println',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Println statement',
            insertText: new vscode.SnippetString('println("${1:message}")'),
            documentation: 'Println to console'
        },
        {
            label: 'print',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Print statement',
            insertText: new vscode.SnippetString('print("${1:message}")'),
            documentation: 'Print to console'
        },
        
        // HTTP Server
        {
            label: 'http-server',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'HTTP server',
            insertText: new vscode.SnippetString('import http\n\nhttp.serve(${1:":8080"}, fn(req) {\n\tresponse.send(${2:"Hello, World!"})\n})'),
            documentation: 'Create an HTTP server'
        },
        
        // Closure
        {
            label: 'closure',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Closure/lambda',
            insertText: new vscode.SnippetString('fn(${1:args}) ${2:=>} ${3:body}'),
            documentation: 'Define a closure/lambda'
        },
        
        // Vector
        {
            label: 'vec',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Vector/array',
            insertText: new vscode.SnippetString('[${1:items}]'),
            documentation: 'Create a vector/array'
        },
        {
            label: 'vec-for',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Vector comprehension',
            insertText: new vscode.SnippetString('[${1:x} for ${2:x} in ${3:collection}]'),
            documentation: 'Vector comprehension'
        },
        
        // Map
        {
            label: 'map',
            kind: vscode.CompletionItemKind.Snippet,
            detail: 'Hash map',
            insertText: new vscode.SnippetString('{${1:"key"}: ${2:value}}'),
            documentation: 'Create a hash map'
        }
    ];
}
