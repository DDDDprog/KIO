import * as vscode from 'vscode';

let replTerminal: vscode.Terminal | undefined;
let replOutputChannel: vscode.OutputChannel | undefined;

export function activateReplProvider(context: vscode.ExtensionContext) {
    // Create output channel for REPL
    replOutputChannel = vscode.window.createOutputChannel('Axeon REPL');
    context.subscriptions.push(replOutputChannel);
    
    // Register REPL terminal command
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.startRepl', async () => {
            const config = vscode.workspace.getConfiguration('axeon');
            const enableRepl = config.get<boolean>('enableRepl', true);
            
            if (!enableRepl) {
                vscode.window.showInformationMessage('REPL is disabled. Enable it in settings.');
                return;
            }
            
            // Show the REPL output channel
            replOutputChannel?.show(true);
            
            // Send welcome message
            replOutputChannel?.appendLine('=== Axeon REPL ===');
            replOutputChannel?.appendLine('Type Axeon code and press Enter to execute.');
            replOutputChannel?.appendLine('Commands: :help, :clear, :exit');
            replOutputChannel?.appendLine('');
            
            // Create interactive input
            const input = await vscode.window.showInputBox({
                prompt: 'Axeon REPL',
                placeHolder: 'Enter code...'
            });
            
            if (input) {
                await executeReplCommand(input);
            }
        })
    );
    
    // Register command to run selected code in REPL
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.runInRepl', async () => {
            const editor = vscode.window.activeTextEditor;
            if (!editor) {
                return;
            }
            
            const selection = editor.selection;
            const selectedText = editor.document.getText(selection);
            
            if (selectedText) {
                replOutputChannel?.show(true);
                replOutputChannel?.appendLine(`> ${selectedText}`);
                await executeReplCommand(selectedText);
            }
        })
    );
    
    // Register debug REPL (interactive debugging)
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.debugRepl', async () => {
            const terminal = vscode.window.createTerminal({
                name: 'Axeon Debug REPL',
                cwd: vscode.workspace.workspaceFolders?.[0]?.uri.fsPath || '.'
            });
            
            terminal.sendText('kio repl --debug');
            terminal.show();
        })
    );
}

async function executeReplCommand(input: string): Promise<void> {
    if (!replOutputChannel) {
        return;
    }
    
    // Handle special commands
    if (input === ':help') {
        replOutputChannel.appendLine('Available commands:');
        replOutputChannel.appendLine('  :help   - Show this help message');
        replOutputChannel.appendLine('  :clear  - Clear REPL output');
        replOutputChannel.appendLine('  :exit   - Close REPL');
        replOutputChannel.appendLine('');
        return;
    }
    
    if (input === ':clear') {
        replOutputChannel.clear();
        return;
    }
    
    if (input === ':exit') {
        replOutputChannel.hide();
        return;
    }
    
    // Execute the code using kio interpreter
    try {
        const terminal = vscode.window.createTerminal({
            name: 'Axeon REPL Runner'
        });
        
        terminal.sendText(`echo "${input.replace(/"/g, '\\"')}" | kio repl`);
        terminal.show();
        
        // Note: This is a basic implementation
        // A full implementation would require a proper REPL server
        replOutputChannel.appendLine('(Executing...)');
        replOutputChannel.appendLine('');
    } catch (error) {
        replOutputChannel.appendLine(`Error: ${error}`);
        replOutputChannel.appendLine('');
    }
}
