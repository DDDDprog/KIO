import * as vscode from 'vscode';
import { spawn } from 'child_process';

export function activate(context: vscode.ExtensionContext) {
  const hello = vscode.commands.registerCommand('kio.hello', () => {
    vscode.window.showInformationMessage('Hello from KIO extension!');
  });

  const formatter = vscode.languages.registerDocumentFormattingEditProvider('kio', {
    provideDocumentFormattingEdits(document: vscode.TextDocument): vscode.TextEdit[] {
      const edits: vscode.TextEdit[] = [];
      const indentUnit = '    ';
      let indentLevel = 0;
      for (let i = 0; i < document.lineCount; i++) {
        const line = document.lineAt(i);
        const text = line.text.trim();
        // decrease indent if line starts with closing brace
        let level = indentLevel;
        if (text.startsWith('}')) {
          level = Math.max(0, indentLevel - 1);
        }
        const desiredIndent = indentUnit.repeat(level);
        const currentIndentMatch = line.text.match(/^\s*/)?.[0] ?? '';
        if (currentIndentMatch !== desiredIndent) {
          const range = new vscode.Range(
            new vscode.Position(i, 0),
            new vscode.Position(i, currentIndentMatch.length)
          );
          edits.push(vscode.TextEdit.replace(range, desiredIndent));
        }
        // adjust indent level for next line
        if (text.endsWith('{')) indentLevel++;
        if (text.endsWith('}')) indentLevel = Math.max(0, indentLevel - 1);
      }
      return edits;
    }
  });

  context.subscriptions.push(hello, formatter);


  // Run current KIO file
  const runFile = vscode.commands.registerCommand('kio.runFile', async () => {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
      vscode.window.showErrorMessage('No active editor');
      return;
    }
    const doc = editor.document;
    if (!doc || !doc.fileName.match(/\.(kio|ko)$/)) {
      vscode.window.showErrorMessage('Current file is not a .kio/.ko file');
      return;
    }
    if (doc.isDirty) {
      await doc.save();
    }

    const output = vscode.window.createOutputChannel('KIO');
    output.clear();
    output.show(true);
    output.appendLine(`Running: kio ${doc.fileName}`);

    const proc = spawn('kio', [doc.fileName], { cwd: vscode.workspace.rootPath || undefined });
    proc.stdout.on('data', (data: Buffer) => output.append(data.toString()));
    proc.stderr.on('data', (data: Buffer) => output.append(data.toString()));
    proc.on('close', (code: number | null) => {
      output.appendLine(`\nProcess exited with code ${code}`);
    });
    proc.on('error', (err: Error) => {
      output.appendLine(`Error: ${err.message}`);
      vscode.window.showErrorMessage('Failed to run kio. Is it installed and on PATH?');
    });
  });

  context.subscriptions.push(runFile);
}

export function deactivate() {}
