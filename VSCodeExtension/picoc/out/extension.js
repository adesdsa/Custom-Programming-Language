"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const vscode = require("vscode");
const child_process_1 = require("child_process");
const path = require("path");
const diagnosticCollection = vscode.languages.createDiagnosticCollection("picoc");
function activate(context) {
    const compileCommand = vscode.commands.registerCommand("picoc.compile", () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor || editor.document.languageId !== "picoc") {
            vscode.window.showErrorMessage("Open a .picoc file first.");
            return;
        }
        editor.document.save().then(() => {
            compilePicoC(editor.document.uri);
        });
    });
    const compileOnSave = vscode.workspace.onDidSaveTextDocument((doc) => {
        const config = vscode.workspace.getConfiguration("picoc");
        if (doc.languageId === "picoc" && config.get("compileOnSave", true)) {
            compilePicoC(doc.uri);
        }
    });
    const hoverProvider = vscode.languages.registerHoverProvider("picoc", { provideHover });
    context.subscriptions.push(compileCommand, compileOnSave, hoverProvider, diagnosticCollection);
}
function compilePicoC(fileUri) {
    const filePath = fileUri.fsPath;
    const config = vscode.workspace.getConfiguration("picoc");
    const compilerPath = config.get("compilerPath", "picoc");
    diagnosticCollection.delete(fileUri);
    const command = `"${compilerPath}" "${filePath}"`;
    const cwd = path.dirname(filePath);
    (0, child_process_1.exec)(command, { cwd }, (error, stdout, stderr) => {
        const output = stdout + "\n" + stderr;
        const diagnostics = parseCompilerOutput(output, fileUri);
        diagnosticCollection.set(fileUri, diagnostics);
        if (diagnostics.length === 0 && !error) {
            vscode.window.showInformationMessage("PicoC: Compiled successfully!");
        }
        else if (diagnostics.length > 0) {
            vscode.window.showErrorMessage(`PicoC: ${diagnostics.length} error(s) found.`);
        }
    });
}
function parseCompilerOutput(output, fileUri) {
    const diagnostics = [];
    const errorPattern = /at file:\s*(.+?)\s*\((\d+),(\d+)\):\s*(.+?):\s*(.+?)(?:\tPROPOSED FIX:\s*(.+))?$/gm;
    let match;
    while ((match = errorPattern.exec(output)) !== null) {
        const line = parseInt(match[2], 10) - 1;
        const col = Math.max(0, parseInt(match[3], 10) - 1);
        const errorType = match[4].trim();
        const message = match[5].trim();
        const hint = match[6]?.trim();
        const range = new vscode.Range(line, col, line, Math.max(col + 1, 1));
        const severity = errorType.toLowerCase().includes("warning")
            ? vscode.DiagnosticSeverity.Warning
            : vscode.DiagnosticSeverity.Error;
        const fullMessage = hint ? `${message} (fix: ${hint})` : message;
        const diagnostic = new vscode.Diagnostic(range, fullMessage, severity);
        diagnostic.source = "picoc";
        diagnostic.code = errorType;
        diagnostics.push(diagnostic);
    }
    return diagnostics;
}
function provideHover(document, position) {
    const wordRange = document.getWordRangeAtPosition(position);
    if (!wordRange)
        return null;
    const word = document.getText(wordRange);
    if (!/^[a-zA-Z_][a-zA-Z0-9_]*$/.test(word))
        return null;
    const text = document.getText();
    const lines = document.getText().split(/\r?\n/);
    // Find a function declaration or definition whose name matches the word
    // Pattern: name(params) -> returnType  [optional ; or newline then {]
    const funcRegex = new RegExp(`\\b(${escapeRegex(word)})\\s*\\(([^)]*)\\)\\s*->\\s*([^;{\\n]+)`, "g");
    let match;
    const candidates = [];
    while ((match = funcRegex.exec(text)) !== null) {
        const name = match[1];
        const params = match[2].trim();
        const returnType = match[3].trim().replace(/\s*[;{]\s*$/, "").trim();
        const beforeMatch = text.substring(0, match.index);
        const lineIndex = (beforeMatch.match(/\n/g) || []).length;
        const signature = `${name}(${params}) -> ${returnType}`;
        const doc = getDocAbove(lines, lineIndex);
        candidates.push({ name, signature, params, returnType, doc });
    }
    const found = candidates.length === 0
        ? null
        : candidates.find((c) => c.doc.length > 0) ?? candidates[0];
    if (!found)
        return null;
    const parts = [];
    parts.push("```picoc\n" + found.signature + "\n```");
    if (found.params && found.params !== "void") {
        parts.push("**Parameters:** " + found.params);
    }
    parts.push("**Returns:** " + found.returnType);
    if (found.doc) {
        parts.push("---\n" + found.doc);
    }
    const content = new vscode.MarkdownString(parts.join("\n\n"));
    content.isTrusted = true;
    return new vscode.Hover(content, wordRange);
}
function escapeRegex(s) {
    return s.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}
/** Collect documentation lines immediately above the given line (0-based). */
function getDocAbove(lines, functionLineIndex) {
    const docLines = [];
    let i = functionLineIndex - 1;
    while (i >= 0) {
        const line = lines[i];
        const trimmed = line.trim();
        // Block comment end: /* ... */ on same line - take content
        const blockMatch = trimmed.match(/\/\*\s*(.*?)\s*\*\/\s*$/);
        if (blockMatch) {
            docLines.unshift(blockMatch[1].trim());
            i--;
            continue;
        }
        // Line comment: # ...
        if (trimmed.startsWith("#")) {
            docLines.unshift(trimmed.slice(1).trim());
            i--;
            continue;
        }
        // Continuation of block comment: ... */
        if (trimmed.endsWith("*/")) {
            const content = trimmed.slice(0, -2).trim();
            if (content)
                docLines.unshift(content);
            i--;
            continue;
        }
        // Start of block comment: /* ...
        if (trimmed.startsWith("/*")) {
            docLines.unshift(trimmed.slice(2).trim());
            i--;
            break;
        }
        // Empty line: still part of doc block
        if (trimmed === "") {
            i--;
            continue;
        }
        // Non-empty, non-comment line: stop
        break;
    }
    return docLines.filter((s) => s.length > 0).join("\n");
}
function deactivate() {
    diagnosticCollection.clear();
    diagnosticCollection.dispose();
}
//# sourceMappingURL=extension.js.map