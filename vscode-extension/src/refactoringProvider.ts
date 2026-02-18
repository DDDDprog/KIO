import * as vscode from 'vscode';

export function activateRefactoringProvider(context: vscode.ExtensionContext) {
    // Register rename provider
    const renameProvider = vscode.languages.registerRenameProvider('axeon', {
        provideRenameEdits(document: vscode.TextDocument, position: vscode.Position, newName: string): vscode.WorkspaceEdit | undefined {
            const wordRange = document.getWordRangeAtPosition(position);
            if (!wordRange) {
                return undefined;
            }
            
            const oldName = document.getText(wordRange);
            const edits = new vscode.WorkspaceEdit();
            
            // Find all occurrences in the document
            const text = document.getText();
            const regex = new RegExp(`\\b${oldName}\\b`, 'g');
            let match;
            
            while ((match = regex.exec(text)) !== null) {
                const startPos = document.positionAt(match.index);
                const endPos = document.positionAt(match.index + oldName.length);
                const range = new vscode.Range(startPos, endPos);
                edits.replace(document.uri, range, newName);
            }
            
            // Also search in other open axeon files
            const openDocs = vscode.workspace.textDocuments;
            for (const doc of openDocs) {
                if (doc.languageId === 'axeon' && doc.uri !== document.uri) {
                    const docText = doc.getText();
                    const docRegex = new RegExp(`\\b${oldName}\\b`, 'g');
                    let docMatch;
                    
                    while ((docMatch = docRegex.exec(docText)) !== null) {
                        const startPos = doc.positionAt(docMatch.index);
                        const endPos = doc.positionAt(docMatch.index + oldName.length);
                        const range = new vscode.Range(startPos, endPos);
                        edits.replace(doc.uri, range, newName);
                    }
                }
            }
            
            return edits;
        }
    });
    
    context.subscriptions.push(renameProvider);
    
    // Register code action provider for extract function
    const codeActionProvider = vscode.languages.registerCodeActionsProvider('axeon', {
        provideCodeActions(document: vscode.TextDocument, range: vscode.Range): vscode.CodeAction[] {
            const actions: vscode.CodeAction[] = [];
            const selectedText = document.getText(range);
            
            // Extract to function action
            const extractAction = new vscode.CodeAction(
                'Extract to function',
                vscode.CodeActionKind.RefactorExtract
            );
            extractAction.command = {
                command: 'axeon.extractFunction',
                title: 'Extract to function',
                arguments: [document.uri, range]
            };
            actions.push(extractAction);
            
            // Inline variable action
            const inlineAction = new vscode.CodeAction(
                'Inline variable',
                vscode.CodeActionKind.RefactorInline
            );
            inlineAction.command = {
                command: 'axeon.inlineVariable',
                title: 'Inline variable',
                arguments: [document.uri, range]
            };
            actions.push(inlineAction);
            
            return actions;
        }
    });
    
    context.subscriptions.push(codeActionProvider);
    
    // Register extract function command
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.extractFunction', async (uri: vscode.Uri, range: vscode.Range) => {
            const document = await vscode.workspace.openTextDocument(uri);
            const selectedText = document.getText(range);
            
            const newName = await vscode.window.showInputBox({
                prompt: 'Enter function name',
                value: 'extractedFunction'
            });
            
            if (newName) {
                const edit = new vscode.WorkspaceEdit();
                
                // Add function definition after selection
                const endPos = range.end;
                const newFunc = `\n\nfn ${newName}() {\n    ${selectedText}\n}`;
                edit.insert(uri, endPos, newFunc);
                
                // Replace selection with function call
                edit.replace(uri, range, `${newName}()`);
                
                await vscode.workspace.applyEdit(edit);
            }
        })
    );
    
    // Register inline variable command
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.inlineVariable', async (uri: vscode.Uri, range: vscode.Range) => {
            const document = await vscode.workspace.openTextDocument(uri);
            const selectedText = document.getText(range);
            
            // Simple inline: just replace the variable reference with its value
            // This is a basic implementation
            const edit = new vscode.WorkspaceEdit();
            edit.replace(uri, range, selectedText);
            await vscode.workspace.applyEdit(edit);
        })
    );
    
    // Register rename symbol command
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.renameSymbol', () => {
            const editor = vscode.window.activeTextEditor;
            if (editor && editor.document.languageId === 'axeon') {
                const position = editor.selection.active;
                vscode.commands.executeCommand('editor.action.rename');
            }
        })
    );
}
