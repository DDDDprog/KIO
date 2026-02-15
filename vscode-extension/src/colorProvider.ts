import * as vscode from 'vscode';

const colorDecorationType = vscode.window.createTextEditorDecorationType({
    backgroundColor: new vscode.ThemeColor('editor.findMatchHighlightBackground'),
    borderWidth: '1px',
    borderStyle: 'solid'
});

export function activateColorProvider(context: vscode.ExtensionContext) {
    // Register color decorator for hex color values
    const disposable = vscode.window.onDidChangeActiveTextEditor(editor => {
        if (editor) {
            updateColorDecorations(editor);
        }
    });
    
    // Also update when document changes
    const changeDisposable = vscode.workspace.onDidChangeTextDocument(event => {
        const editor = vscode.window.activeTextEditor;
        if (editor && event.document === editor.document) {
            updateColorDecorations(editor);
        }
    });
    
    context.subscriptions.push(disposable, changeDisposable);
    
    // Initial update
    if (vscode.window.activeTextEditor) {
        updateColorDecorations(vscode.window.activeTextEditor);
    }
}

function updateColorDecorations(editor: vscode.TextEditor) {
    const document = editor.document;
    const text = document.getText();
    const decorations: vscode.DecorationOptions[] = [];
    
    // Parse document for color literals (hex colors like #FF0000)
    const hexColorRegex = /#([0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})\b/g;
    
    let match;
    while ((match = hexColorRegex.exec(text)) !== null) {
        const startPos = document.positionAt(match.index);
        const endPos = document.positionAt(match.index + match[0].length);
        
        const range = new vscode.Range(startPos, endPos);
        const color = parseHexColor(match[0]);
        
        if (color) {
            const rgba = `rgba(${Math.round(color.red * 255)}, ${Math.round(color.green * 255)}, ${Math.round(color.blue * 255)}, ${color.alpha})`;
            decorations.push({
                range: range,
                renderOptions: {
                    before: {
                        backgroundColor: rgba,
                        width: '12px',
                        height: '12px',
                        margin: '0 4px 0 0',
                        border: '1px solid #888'
                    }
                }
            });
        }
    }
    
    editor.setDecorations(colorDecorationType, decorations);
}

function parseHexColor(hex: string): vscode.Color | null {
    try {
        const r = parseInt(hex.slice(1, 3), 16) / 255;
        const g = parseInt(hex.slice(3, 5), 16) / 255;
        const b = parseInt(hex.slice(5, 7), 16) / 255;
        const a = hex.length === 9 ? parseInt(hex.slice(7, 9), 16) / 255 : 1;
        
        return new vscode.Color(r, g, b, a);
    } catch {
        return null;
    }
}
