import * as vscode from 'vscode';
import { LanguageClient, LanguageClientOptions, ServerOptions, TransportKind } from 'vscode-languageclient/node';
import { activateColorProvider } from './colorProvider';
import { activateCompletionProvider } from './completionProvider';

let languageClient: LanguageClient | undefined;

export function activate(context: vscode.ExtensionContext) {
    console.log('Axeon extension is now active!');

    // Check if LSP server path is configured
    const serverPath = vscode.workspace.getConfiguration('axeon').get<string>('lspServerPath');
    
    if (serverPath) {
        // Use external LSP server
        const serverOptions: ServerOptions = {
            run: { command: serverPath },
            debug: { command: serverPath }
        };

        const clientOptions: LanguageClientOptions = {
            documentSelector: [
                { scheme: 'file', language: 'axeon' },
                { scheme: 'file', pattern: '**/*.axe' },
                { scheme: 'file', pattern: '**/*.ko' }
            ],
            synchronize: {
                fileEvents: vscode.workspace.createFileSystemWatcher('**/*.{axe,ko}')
            }
        };

        languageClient = new LanguageClient(
            'axeonLanguageServer',
            'Axeon Language Server',
            serverOptions,
            clientOptions
        );

        languageClient.start();
    } else {
        vscode.window.showInformationMessage('Axeon: Set "axeon.lspServerPath" in settings to enable full LSP support.');
    }

    // Register color provider for syntax highlighting
    activateColorProvider(context);

    // Register completion provider
    activateCompletionProvider(context);

    // Register commands
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.runFile', async () => {
            const editor = vscode.window.activeTextEditor;
            if (editor && (editor.document.languageId === 'axeon' || 
                          editor.document.fileName.endsWith('.axe') ||
                          editor.document.fileName.endsWith('.ko'))) {
                const document = editor.document;
                const filePath = document.uri.fsPath;
                
                const terminal = vscode.window.createTerminal({
                    name: `Run ${document.fileName.split(/[\\/]/).pop()}`,
                    cwd: vscode.workspace.workspaceFolders?.[0]?.uri.fsPath || '.'
                });
                
                terminal.sendText(`kio run "${filePath}"`);
                terminal.show();
            }
        })
    );

    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.showVersion', () => {
            vscode.window.showInformationMessage('Axeon Language Support v1.0.0');
        })
    );
}

export function deactivate() {
    if (languageClient) {
        languageClient.stop();
    }
}
