"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const vscode = require("vscode");
// --- Comment tag decoration types ---
const commentTagDecorations = {
    todo: vscode.window.createTextEditorDecorationType({
        color: "#000000",
        backgroundColor: "#FF8C00",
        fontWeight: "bold",
        rangeBehavior: vscode.DecorationRangeBehavior.ClosedClosed,
        overviewRulerColor: "#FF8C00",
        overviewRulerLane: vscode.OverviewRulerLane.Center,
    }),
    fixme: vscode.window.createTextEditorDecorationType({
        color: "#000000",
        backgroundColor: "#FF69B4",
        fontWeight: "bold",
        rangeBehavior: vscode.DecorationRangeBehavior.ClosedClosed,
        overviewRulerColor: "#FF69B4",
        overviewRulerLane: vscode.OverviewRulerLane.Center,
    }),
    alert: vscode.window.createTextEditorDecorationType({
        color: "#FF2D00",
        fontWeight: "bold",
    }),
    question: vscode.window.createTextEditorDecorationType({
        color: "#3498DB",
        fontStyle: "italic",
    }),
    highlight: vscode.window.createTextEditorDecorationType({
        color: "#98C379",
        fontWeight: "bold",
    }),
    docTag: vscode.window.createTextEditorDecorationType({
        color: "#C586C0",
        fontWeight: "bold",
    }),
    docParamName: vscode.window.createTextEditorDecorationType({
        color: "#9CDCFE",
        fontStyle: "italic",
    }),
};
function updateCommentDecorations(editor) {
    if (editor.document.languageId !== "picoc")
        return;
    const text = editor.document.getText();
    const buckets = {
        todo: [], fixme: [], alert: [], question: [], highlight: [],
        docTag: [], docParamName: [],
    };
    const commentRegex = /#(.*)$|\/\*([\s\S]*?)\*\//gm;
    let cm;
    while ((cm = commentRegex.exec(text)) !== null) {
        const commentBody = cm[1] ?? cm[2];
        const bodyStart = cm[1] !== undefined ? cm.index + 1 : cm.index + 2;
        scanCommentBody(editor, commentBody, bodyStart, buckets);
    }
    editor.setDecorations(commentTagDecorations.todo, buckets.todo);
    editor.setDecorations(commentTagDecorations.fixme, buckets.fixme);
    editor.setDecorations(commentTagDecorations.alert, buckets.alert);
    editor.setDecorations(commentTagDecorations.question, buckets.question);
    editor.setDecorations(commentTagDecorations.highlight, buckets.highlight);
    editor.setDecorations(commentTagDecorations.docTag, buckets.docTag);
    editor.setDecorations(commentTagDecorations.docParamName, buckets.docParamName);
}
const DOC_TAGS = ["@brief", "@param", "@return", "@returns", "@note", "@warning", "@see", "@example", "@deprecated", "@throws", "@author", "@version", "@since"];
function scanCommentBody(editor, body, offset, buckets) {
    const tagPatterns = [
        { regex: /\bTODO\b:?/g, target: buckets.todo },
        { regex: /\bFIXME\b:?/g, target: buckets.fixme },
        { regex: /!\s.+/g, target: buckets.alert },
        { regex: /\?\s.+/g, target: buckets.question },
        { regex: /\*\s.+/g, target: buckets.highlight },
    ];
    for (const { regex, target } of tagPatterns) {
        let m;
        while ((m = regex.exec(body)) !== null) {
            const startPos = editor.document.positionAt(offset + m.index);
            const endPos = editor.document.positionAt(offset + m.index + m[0].length);
            target.push({ range: new vscode.Range(startPos, endPos) });
        }
    }
    // @tag highlighting
    const docTagRegex = /@(?:brief|param|returns?|note|warning|see|example|deprecated|throws|author|version|since)\b/g;
    let dt;
    while ((dt = docTagRegex.exec(body)) !== null) {
        const startPos = editor.document.positionAt(offset + dt.index);
        const endPos = editor.document.positionAt(offset + dt.index + dt[0].length);
        buckets.docTag.push({ range: new vscode.Range(startPos, endPos) });
        // For @param, highlight the parameter name that follows
        if (dt[0] === "@param") {
            const afterTag = body.substring(dt.index + dt[0].length);
            const nameMatch = afterTag.match(/^\s+([a-zA-Z_][a-zA-Z0-9_]*)/);
            if (nameMatch) {
                const nameStart = dt.index + dt[0].length + nameMatch.index + nameMatch[0].indexOf(nameMatch[1]);
                const sPos = editor.document.positionAt(offset + nameStart);
                const ePos = editor.document.positionAt(offset + nameStart + nameMatch[1].length);
                buckets.docParamName.push({ range: new vscode.Range(sPos, ePos) });
            }
        }
    }
}
function activate(context) {
    const hoverProvider = vscode.languages.registerHoverProvider("picoc", { provideHover });
    const completionProvider = vscode.languages.registerCompletionItemProvider("picoc", { provideCompletionItems }, ".");
    const docTagCompletionProvider = vscode.languages.registerCompletionItemProvider("picoc", { provideCompletionItems: provideDocTagCompletions }, "@");
    const definitionProvider = vscode.languages.registerDefinitionProvider("picoc", { provideDefinition });
    // Trigger decoration updates on editor changes
    if (vscode.window.activeTextEditor) {
        updateCommentDecorations(vscode.window.activeTextEditor);
    }
    const onEditorChange = vscode.window.onDidChangeActiveTextEditor((editor) => {
        if (editor)
            updateCommentDecorations(editor);
    });
    const onDocChange = vscode.workspace.onDidChangeTextDocument((event) => {
        const editor = vscode.window.activeTextEditor;
        if (editor && event.document === editor.document) {
            updateCommentDecorations(editor);
        }
    });
    context.subscriptions.push(hoverProvider, completionProvider, docTagCompletionProvider, definitionProvider, onEditorChange, onDocChange, ...Object.values(commentTagDecorations));
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
    if (found.doc) {
        const parsed = parseDocTags(found.doc);
        if (parsed.brief) {
            parts.push(parsed.brief);
        }
        if (parsed.params.length > 0) {
            parts.push("**Parameters:**\n" + parsed.params.map((p) => `- \`${p.name}\` — ${p.desc}`).join("\n"));
        }
        else if (found.params && found.params !== "void") {
            parts.push("**Parameters:** " + found.params);
        }
        if (parsed.returns) {
            parts.push("**Returns:** " + parsed.returns);
        }
        else {
            parts.push("**Returns:** " + found.returnType);
        }
        if (parsed.notes.length > 0) {
            parts.push(parsed.notes.map((n) => "**Note:** " + n).join("\n\n"));
        }
        if (parsed.warnings.length > 0) {
            parts.push(parsed.warnings.map((w) => "**Warning:** " + w).join("\n\n"));
        }
        if (parsed.deprecated) {
            parts.push("**Deprecated:** " + parsed.deprecated);
        }
        if (parsed.throws.length > 0) {
            parts.push("**Throws:** " + parsed.throws.join(", "));
        }
        if (parsed.see.length > 0) {
            parts.push("**See:** " + parsed.see.join(", "));
        }
        if (parsed.example) {
            parts.push("**Example:**\n```picoc\n" + parsed.example + "\n```");
        }
        if (parsed.other) {
            parts.push("---\n" + parsed.other);
        }
    }
    else {
        if (found.params && found.params !== "void") {
            parts.push("**Parameters:** " + found.params);
        }
        parts.push("**Returns:** " + found.returnType);
    }
    const content = new vscode.MarkdownString(parts.join("\n\n"));
    content.isTrusted = true;
    return new vscode.Hover(content, wordRange);
}
function parseDocTags(doc) {
    const result = {
        brief: "", params: [], returns: "", notes: [], warnings: [],
        deprecated: "", throws: [], see: [], example: "", other: "",
    };
    const lines = doc.split("\n");
    const otherLines = [];
    let i = 0;
    while (i < lines.length) {
        const line = lines[i].trim();
        const tagMatch = line.match(/^@(\w+)\s*(.*)/);
        if (!tagMatch) {
            otherLines.push(line);
            i++;
            continue;
        }
        const tag = tagMatch[1].toLowerCase();
        let value = tagMatch[2].trim();
        i++;
        // Collect continuation lines (not starting with @)
        while (i < lines.length && !lines[i].trim().startsWith("@")) {
            value += " " + lines[i].trim();
            i++;
        }
        value = value.trim();
        switch (tag) {
            case "brief":
                result.brief = value;
                break;
            case "param": {
                const pm = value.match(/^([a-zA-Z_][a-zA-Z0-9_]*)\s+(.*)/);
                if (pm) {
                    result.params.push({ name: pm[1], desc: pm[2] });
                }
                else {
                    result.params.push({ name: value, desc: "" });
                }
                break;
            }
            case "return":
            case "returns":
                result.returns = value;
                break;
            case "note":
                result.notes.push(value);
                break;
            case "warning":
                result.warnings.push(value);
                break;
            case "deprecated":
                result.deprecated = value || "This function is deprecated.";
                break;
            case "throws":
                result.throws.push(value);
                break;
            case "see":
                result.see.push(value);
                break;
            case "example":
                result.example = value;
                break;
            default:
                otherLines.push(line);
                break;
        }
    }
    result.other = otherLines.filter((s) => s.length > 0).join("\n");
    return result;
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
// --- Auto-completion provider ---
const KEYWORD_COMPLETIONS = [
    "if", "elif", "else", "for", "while", "break", "continue", "return", "pass",
    "and", "or", "not", "xor", "sizeof",
].map((kw) => {
    const item = new vscode.CompletionItem(kw, vscode.CompletionItemKind.Keyword);
    item.detail = "keyword";
    return item;
});
const TYPE_COMPLETIONS = [
    "void", "bool", "int", "double", "unsigned",
].map((t) => {
    const item = new vscode.CompletionItem(t, vscode.CompletionItemKind.TypeParameter);
    item.detail = "type";
    return item;
});
const MODIFIER_COMPLETIONS = [
    "let", "var", "const", "readonly", "global",
].map((m) => {
    const item = new vscode.CompletionItem(m, vscode.CompletionItemKind.Keyword);
    item.detail = "modifier";
    return item;
});
const BUILTIN_COMPLETIONS = [
    { name: "println", sig: "println(value)" },
    { name: "sizeof", sig: "sizeof(type_or_expr)" },
    { name: "true", sig: "true" },
    { name: "false", sig: "false" },
].map((b) => {
    const item = new vscode.CompletionItem(b.name, b.name === "true" || b.name === "false"
        ? vscode.CompletionItemKind.Constant
        : vscode.CompletionItemKind.Function);
    item.detail = "built-in";
    item.documentation = b.sig;
    return item;
});
function provideCompletionItems(document, position) {
    const items = [
        ...KEYWORD_COMPLETIONS,
        ...TYPE_COMPLETIONS,
        ...MODIFIER_COMPLETIONS,
        ...BUILTIN_COMPLETIONS,
    ];
    const text = document.getText();
    // Collect function names from definitions and forward declarations
    const funcRegex = /\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\(([^)]*)\)\s*->\s*([^;{\n]+)/g;
    const seenFunctions = new Set();
    let m;
    while ((m = funcRegex.exec(text)) !== null) {
        const name = m[1];
        if (seenFunctions.has(name))
            continue;
        seenFunctions.add(name);
        const params = m[2].trim();
        const retType = m[3].trim().replace(/\s*[;{]\s*$/, "").trim();
        const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Function);
        item.detail = `(${params}) -> ${retType}`;
        item.insertText = new vscode.SnippetString(`${name}(\${1})`);
        items.push(item);
    }
    // Collect variable names from let/var/global declarations
    const varRegex = /\b(?:let(?:\s+(?:const|readonly))?|var|global)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*:\s*([a-zA-Z_][a-zA-Z0-9_ ]*)/g;
    const seenVars = new Set();
    while ((m = varRegex.exec(text)) !== null) {
        const name = m[1];
        if (seenVars.has(name))
            continue;
        seenVars.add(name);
        const varType = m[2].trim();
        const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Variable);
        item.detail = varType;
        items.push(item);
    }
    // Collect parameter names from function signatures
    const paramBlockRegex = /\b[a-zA-Z_][a-zA-Z0-9_]*\s*\(([^)]+)\)\s*->/g;
    while ((m = paramBlockRegex.exec(text)) !== null) {
        const paramStr = m[1];
        const paramParts = paramStr.split(",");
        for (const part of paramParts) {
            const pm = part.trim().match(/^([a-zA-Z_][a-zA-Z0-9_]*)\s*:\s*([a-zA-Z_][a-zA-Z0-9_ ]*)/);
            if (pm && !seenVars.has(pm[1]) && pm[1] !== "void") {
                seenVars.add(pm[1]);
                const item = new vscode.CompletionItem(pm[1], vscode.CompletionItemKind.Variable);
                item.detail = pm[2].trim() + " (parameter)";
                items.push(item);
            }
        }
    }
    return items;
}
// --- Doc-tag completion provider (triggered by @) ---
const DOC_TAG_COMPLETIONS = [
    { label: "@brief", detail: "Brief description of the function", snippet: "@brief ${1:description}" },
    { label: "@param", detail: "Describe a parameter", snippet: "@param ${1:name} ${2:description}" },
    { label: "@return", detail: "Describe the return value", snippet: "@return ${1:description}" },
    { label: "@returns", detail: "Describe the return value", snippet: "@returns ${1:description}" },
    { label: "@note", detail: "Additional note", snippet: "@note ${1:text}" },
    { label: "@warning", detail: "Warning about usage", snippet: "@warning ${1:text}" },
    { label: "@see", detail: "Reference to related item", snippet: "@see ${1:reference}" },
    { label: "@example", detail: "Usage example", snippet: "@example ${1:code}" },
    { label: "@deprecated", detail: "Mark as deprecated", snippet: "@deprecated ${1:reason}" },
    { label: "@throws", detail: "Describe an error condition", snippet: "@throws ${1:description}" },
    { label: "@author", detail: "Author of this code", snippet: "@author ${1:name}" },
    { label: "@version", detail: "Version information", snippet: "@version ${1:version}" },
    { label: "@since", detail: "Version when this was introduced", snippet: "@since ${1:version}" },
];
function provideDocTagCompletions(document, position) {
    const lineText = document.lineAt(position).text;
    const prefix = lineText.substring(0, position.character);
    // Only offer @-tag completions inside comments
    const inLineComment = prefix.includes("#");
    const textBefore = document.getText(new vscode.Range(new vscode.Position(0, 0), position));
    const openBlock = textBefore.lastIndexOf("/*");
    const closeBlock = textBefore.lastIndexOf("*/");
    const inBlockComment = openBlock > closeBlock;
    if (!inLineComment && !inBlockComment)
        return null;
    // Find where the @ starts so we can replace from there (avoids @@)
    const atIndex = prefix.lastIndexOf("@");
    const replaceStart = new vscode.Position(position.line, atIndex >= 0 ? atIndex : position.character);
    const replaceRange = new vscode.Range(replaceStart, position);
    return DOC_TAG_COMPLETIONS.map((tag) => {
        const item = new vscode.CompletionItem(tag.label, vscode.CompletionItemKind.Keyword);
        item.detail = tag.detail;
        item.insertText = new vscode.SnippetString(tag.snippet);
        item.filterText = tag.label;
        item.sortText = "0" + tag.label;
        item.range = replaceRange;
        return item;
    });
}
// --- Go to Definition provider ---
function provideDefinition(document, position) {
    const wordRange = document.getWordRangeAtPosition(position);
    if (!wordRange)
        return null;
    const word = document.getText(wordRange);
    if (!/^[a-zA-Z_][a-zA-Z0-9_]*$/.test(word))
        return null;
    const text = document.getText();
    const lines = text.split(/\r?\n/);
    const locations = [];
    // Look for function definitions (with a body `{`, not just forward declarations ending in `;`)
    const funcDefRegex = new RegExp(`\\b(${escapeRegex(word)})\\s*\\([^)]*\\)\\s*->\\s*[^;{\\n]+\\s*\\{`, "g");
    let m;
    while ((m = funcDefRegex.exec(text)) !== null) {
        const before = text.substring(0, m.index);
        const line = (before.match(/\n/g) || []).length;
        const col = m.index - (before.lastIndexOf("\n") + 1);
        locations.push(new vscode.Location(document.uri, new vscode.Position(line, col)));
    }
    // If no definition with body found, fall back to forward declarations
    if (locations.length === 0) {
        const funcDeclRegex = new RegExp(`\\b(${escapeRegex(word)})\\s*\\([^)]*\\)\\s*->\\s*[^;{\\n]+;`, "g");
        while ((m = funcDeclRegex.exec(text)) !== null) {
            const before = text.substring(0, m.index);
            const line = (before.match(/\n/g) || []).length;
            const col = m.index - (before.lastIndexOf("\n") + 1);
            locations.push(new vscode.Location(document.uri, new vscode.Position(line, col)));
        }
    }
    // Look for variable declarations: let/var/global <word>
    const varDeclRegex = new RegExp(`\\b(?:let(?:\\s+(?:const|readonly))?|var|global)\\s+(${escapeRegex(word)})\\s*:`, "g");
    while ((m = varDeclRegex.exec(text)) !== null) {
        const before = text.substring(0, m.index + m[0].indexOf(word));
        const line = (before.match(/\n/g) || []).length;
        const col = before.length - (before.lastIndexOf("\n") + 1);
        locations.push(new vscode.Location(document.uri, new vscode.Position(line, col)));
    }
    if (locations.length === 0)
        return null;
    return locations.length === 1 ? locations[0] : locations;
}
function deactivate() { }
//# sourceMappingURL=extension.js.map