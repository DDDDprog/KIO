import * as vscode from 'vscode';

export function activateLiveShareProvider(context: vscode.ExtensionContext) {
    // Check if Live Share is installed
    const liveShareExtension = vscode.extensions.getExtension('ms-vsliveshare.vsliveshare');
    
    if (!liveShareExtension) {
        // Live Share not installed, show notification
        context.subscriptions.push(
            vscode.commands.registerCommand('axeon.startLiveShare', async () => {
                const choice = await vscode.window.showInformationMessage(
                    'Visual Studio Live Share is required for collaboration features.',
                    'Install Live Share',
                    'Dismiss'
                );
                
                if (choice === 'Install Live Share') {
                    vscode.commands.executeCommand('vscode.open', vscode.Uri.parse(
                        'https://marketplace.visualstudio.com/items?itemName=ms-vsliveshare.vsliveshare'
                    ));
                }
            })
        );
        return;
    }
    
    // Register Live Share commands
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.startLiveShare', async () => {
            try {
                // Start a Live Share session
                await vscode.commands.executeCommand('liveshare.start');
                
                vscode.window.showInformationMessage('Axeon: Live Share session started!');
                
                // Share the current workspace
                const terminal = vscode.window.createTerminal({
                    name: 'Axeon Live Share'
                });
                terminal.sendText('echo "Live Share session active for Axeon development"');
                terminal.show();
            } catch (error) {
                vscode.window.showErrorMessage(`Failed to start Live Share: ${error}`);
            }
        })
    );
    
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.joinLiveShare', async () => {
            const joinLink = await vscode.window.showInputBox({
                prompt: 'Enter Live Share invitation link',
                placeHolder: 'vsls://...'
            });
            
            if (joinLink) {
                try {
                    await vscode.commands.executeCommand('liveshare.join', vscode.Uri.parse(joinLink));
                    vscode.window.showInformationMessage('Axeon: Joined Live Share session!');
                } catch (error) {
                    vscode.window.showErrorMessage(`Failed to join Live Share: ${error}`);
                }
            }
        })
    );
    
    // Register terminal provider for shared REPL
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.shareRepl', async () => {
            // Create a shared terminal for collaborative coding
            const terminal = vscode.window.createTerminal({
                name: 'Axeon Shared REPL'
            });
            
            terminal.sendText('kio repl --shared');
            terminal.show();
            
            vscode.window.showInformationMessage(
                'Axeon: Shared REPL terminal created. Collaborators can now code together!'
            );
        })
    );
    
    // Add session status indicator
    context.subscriptions.push(
        vscode.commands.registerCommand('axeon.liveShareStatus', async () => {
            try {
                const session = await vscode.commands.executeCommand<any>('liveshare.getSession');
                if (session) {
                    vscode.window.showInformationMessage(
                        `Live Share: ${session.state} - ${session.sessionId}`
                    );
                } else {
                    vscode.window.showInformationMessage('No active Live Share session');
                }
            } catch {
                vscode.window.showInformationMessage('Live Share is not active');
            }
        })
    );
}
