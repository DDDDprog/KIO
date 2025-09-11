# KIO Language Support for VS Code

<img src="images/logo.jpg" alt="KIO Logo" width="160" />

A lightweight, fast VS Code extension that provides a great editing experience for the KIO language.

## Features

- Syntax highlighting for `.kio` and `.ko`
- Comment toggling and bracket pairs
- Operator and keyword highlighting
- Starter code snippets (type a prefix and press Tab)
  - `kio-print` — print
  - `kio-let` — variable declaration
  - `kio-if` — if/else block
  - `kio-while` — while loop

> Roadmap: formatting, hover/completion via LSP, semantic tokens.

## Snippets

Snippets are defined in `snippets/kio.json`.

- `kio-print`
```kio
print "$1";
```

- `kio-let`
```kio
let ${1:name} = ${2:value};
```

- `kio-if`
```kio
if (${1:condition}) {
    $0
} else {
    
}
```

- `kio-while`
```kio
while (${1:condition}) {
    $0
}
```

## How to Run (Development)

1. Open this folder (`vscode-extension/`) in VS Code.
2. Press `F5` to launch the Extension Development Host.
3. Open a `.kio` file to see highlighting and use snippets.

## Packaging and Installing

```bash
# Install vsce if needed
npm install -g vsce
# From inside vscode-extension/
vsce package
```

Then, in VS Code: Extensions panel → `⋯` → `Install from VSIX…` and pick the generated file.

## Troubleshooting

- If the icon doesn’t render in the VS Code marketplace: make sure `images/logo.jpg` exists in this folder (see below).

## Assets

- Logo is expected at `images/logo.jpg` inside the extension folder. If you have the logo in the repo root at `docs/image.jpg`, copy it like this:

```bash
mkdir -p vscode-extension/images
cp docs/image.jpg vscode-extension/images/logo.jpg
```
