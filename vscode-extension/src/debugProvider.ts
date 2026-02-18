import * as vscode from 'vscode';

export function activateDebugProvider(context: vscode.ExtensionContext) {
    // Register debug configuration provider
    const provider = vscode.debug.registerDebugConfigurationProvider('axeon', {
        provideDebugConfigurations: (folder?: vscode.WorkspaceFolder): vscode.DebugConfiguration[] => {
            return [
                {
                    name: 'Debug Axeon File',
                    type: 'axeon',
                    request: 'launch',
                    program: '${file}',
                    stopOnEntry: true,
                    args: []
                },
                {
                    name: 'Debug Current File',
                    type: 'axeon',
                    request: 'launch',
                    program: '${workspaceFolder}/${fileBasename}',
                    stopOnEntry: true,
                    args: []
                }
            ];
        },
        
        resolveDebugConfiguration: (
            folder: vscode.WorkspaceFolder | undefined,
            config: vscode.DebugConfiguration
        ): vscode.DebugConfiguration | undefined => {
            if (!config.program && folder) {
                // Use the first .axe file in the workspace
                const axeFiles = vscode.workspace.findFiles('*.axe', '**/node_modules/**');
                // We'll handle this in the debug session
            }
            
            // Ensure required fields exist
            if (!config.type) {
                config.type = 'axeon';
            }
            if (!config.request) {
                config.request = 'launch';
            }
            
            return config;
        }
    });
    
    context.subscriptions.push(provider);
    
    // Register debug adapter executable factory
    context.subscriptions.push(
        vscode.debug.registerDebugAdapterDescriptorFactory('axeon', {
            createDebugAdapterDescriptor: (session: vscode.DebugSession, executable: vscode.DebugAdapterExecutable | undefined) => {
                // Use kio with debug flags
                const command = 'kio';
                const args = ['debug', '--adapter'];
                
                return new vscode.DebugAdapterExecutable(command, args);
            }
        })
    );
    
    // Register commands for debugging
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.debugFile', async () => {
            const editor = vscode.window.activeTextEditor;
            if (!editor) {
                return;
            }
            
            const filePath = editor.document.uri.fsPath;
            const folder = vscode.workspace.getWorkspaceFolder(editor.document.uri);
            
            if (!folder) {
                vscode.window.showErrorMessage('Please open a workspace to debug.');
                return;
            }
            
            // Start debugging
            const debugConfig: vscode.DebugConfiguration = {
                name: 'Debug ' + editor.document.fileName.split(/[\\/]/).pop(),
                type: 'axeon',
                request: 'launch',
                program: filePath,
                stopOnEntry: true,
                args: []
            };
            
            vscode.debug.startDebugging(folder, debugConfig);
        })
    );
    
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.toggleBreakpoint', async () => {
            const editor = vscode.window.activeTextEditor;
            if (!editor) {
                return;
            }
            
            // Toggle breakpoint at current line
            vscode.commands.executeCommand('editor.debug.action.toggleBreakpoint');
        })
    );
    
    // Add debug toolbar buttons
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.debugStepOver', () => {
            vscode.commands.executeCommand('workbench.actions.view.debug');
            vscode.commands.executeCommand('debug.action.stepOver');
        })
    );
    
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.debugStepInto', () => {
            vscode.commands.executeCommand('debug.action.stepInto');
        })
    );
    
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.debugStepOut', () => {
            vscode.commands.executeCommand('debug.action.stepOut');
        })
    );
    
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.debugContinue', () => {
            vscode.commands.executeCommand('debug.action.continue');
        })
    );
    
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.debugStop', () => {
            vscode.commands.executeCommand('debug.action.stop');
        })
    );
    
    // Debug console commands
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.evaluateInDebug', async () => {
            const result = await vscode.window.showInputBox({
                prompt: 'Evaluate expression in debug console',
                placeHolder: 'Enter expression...'
            });
            
            if (result) {
                // Execute in debug console
                vscode.commands.executeCommand('debug.console.input', result);
            }
        })
    );
}
