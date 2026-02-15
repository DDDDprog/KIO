# Axeon Language Support for VS Code

A VS Code extension providing language support for the Axeon programming language.

## Features

- **Syntax Highlighting**: Full syntax highlighting for `.axe` and `.ko` files
- **IntelliSense**: Auto-completion for keywords, built-in functions, and snippets
- **Color Picker**: Color preview for hex color values
- **Code Snippets**: Common code patterns for faster development
- **Run Command**: Run Axeon files directly from VS Code

## Installation

### From Source

1. Navigate to the extension directory:
   ```bash
   cd vscode-extension
 Install dependencies:
   ```bash
   npm install
     ```

2. ```

3. Compile TypeScript:
   ```bash
   npm run compile
   ```

4. Press `F5` in VS Code to launch the extension in development mode

### From Package

1. Build the extension:
   ```bash
   npm install -g vsce
   vsce package
   ```

2. Install the `.vsix` file:
   ```bash
   code --install-extension axeon-language-*.vsix
   ```

## Configuration

The extension provides the following settings:

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `axeon.enable` | boolean | true | Enable/disable extension |
| `axeon.lspServerPath` | string | "" | Path to LSP server executable |
| `axeon.trace.server` | string | "off" | Trace LSP communication |

### Setting LSP Server Path

To enable full LSP support (goto definition, hover, etc.), set the path to your LSP server:

```json
{
  "axeon.lspServerPath": "/path/to/kio-lsp"
}
```

Build the LSP server from the `src/lsp/` directory.

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `F5` | Run current file (when in development) |

## Supported File Extensions

- `.axe` - Main Axeon source files
- `.ko` - Module/package files

## Contributing

Contributions are welcome! Please read the contributing guidelines before submitting PRs.

## License

MIT License
