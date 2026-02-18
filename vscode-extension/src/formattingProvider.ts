import * as vscode from 'vscode';

export function activateFormattingProvider(context: vscode.ExtensionContext) {
    // Register document formatting provider
    const formatter = vscode.languages.registerDocumentFormattingEditProvider('axeon', {
        provideDocumentFormattingEdits(document: vscode.TextDocument): vscode.TextEdit[] {
            const edits: vscode.TextEdit[] = [];
            const fullText = document.getText();
            
            // Simple formatting: add proper indentation
            const lines = fullText.split('\n');
            let indentLevel = 0;
            const indentSize = 4;
            const formattedLines: string[] = [];
            
            for (let i = 0; i < lines.length; i++) {
                let line = lines[i].trim();
                
                // Decrease indent for closing braces/brackets
                if (line.match(/^[\}\]]/)) {
                    indentLevel = Math.max(0, indentLevel - 1);
                }
                
                // Add indentation
                if (line.length > 0) {
                    const indent = ' '.repeat(indentLevel * indentSize);
                    formattedLines.push(indent + line);
                } else {
                    formattedLines.push('');
                }
                
                // Increase indent for opening braces/brackets
                if (line.match(/[\{\[]$/)) {
                    indentLevel++;
                }
            }
            
            // Replace entire document
            const fullRange = new vscode.Range(
                document.positionAt(0),
                document.positionAt(fullText.length)
            );
            
            edits.push(vscode.TextEdit.replace(fullRange, formattedLines.join('\n')));
            
            return edits;
        }
    });
    
    context.subscriptions.push(formatter);
    
    // Register command for manual formatting
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.formatDocument', async () => {
            const editor = vscode.window.activeTextEditor;
            if (editor) {
                const document = editor.document;
                if (document.languageId === 'axeon') {
                    const edits = await vscode.commands.executeCommand<vscode.TextEdit[]>(
                        'vscode.executeDocumentFormattingProvider',
                        document.uri
                    );
                    if (edits) {
                        const workspaceEdit = new vscode.WorkspaceEdit();
                        workspaceEdit.set(document.uri, edits);
                        await vscode.workspace.applyEdit(workspaceEdit);
                    }
                }
            }
        })
    );
}
