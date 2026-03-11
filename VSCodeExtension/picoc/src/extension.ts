import * as vscode from "vscode";
import * as path from "path";
import * as fs from "fs";

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

interface DecorationBuckets {
  todo: vscode.DecorationOptions[];
  fixme: vscode.DecorationOptions[];
  alert: vscode.DecorationOptions[];
  question: vscode.DecorationOptions[];
  highlight: vscode.DecorationOptions[];
  docTag: vscode.DecorationOptions[];
  docParamName: vscode.DecorationOptions[];
}

function updateCommentDecorations(editor: vscode.TextEditor): void {
  if (editor.document.languageId !== "picoc") return;

  const text = editor.document.getText();
  const buckets: DecorationBuckets = {
    todo: [], fixme: [], alert: [], question: [], highlight: [],
    docTag: [], docParamName: [],
  };

  const commentRegex = /#(.*)$|\/\*([\s\S]*?)\*\//gm;
  let cm: RegExpExecArray | null;
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

function scanCommentBody(
  editor: vscode.TextEditor,
  body: string,
  offset: number,
  buckets: DecorationBuckets
): void {
  const tagPatterns: { regex: RegExp; target: vscode.DecorationOptions[] }[] = [
    { regex: /\bTODO\b:?/g, target: buckets.todo },
    { regex: /\bFIXME\b:?/g, target: buckets.fixme },
    { regex: /!\s.+/g, target: buckets.alert },
    { regex: /\?\s.+/g, target: buckets.question },
    { regex: /\*\s.+/g, target: buckets.highlight },
  ];

  for (const { regex, target } of tagPatterns) {
    let m: RegExpExecArray | null;
    while ((m = regex.exec(body)) !== null) {
      const startPos = editor.document.positionAt(offset + m.index);
      const endPos = editor.document.positionAt(offset + m.index + m[0].length);
      target.push({ range: new vscode.Range(startPos, endPos) });
    }
  }

  // @tag highlighting
  const docTagRegex = /@(?:brief|param|returns?|note|warning|see|example|deprecated|throws|author|version|since)\b/g;
  let dt: RegExpExecArray | null;
  while ((dt = docTagRegex.exec(body)) !== null) {
    const startPos = editor.document.positionAt(offset + dt.index);
    const endPos = editor.document.positionAt(offset + dt.index + dt[0].length);
    buckets.docTag.push({ range: new vscode.Range(startPos, endPos) });

    // For @param, highlight the parameter name that follows
    if (dt[0] === "@param") {
      const afterTag = body.substring(dt.index + dt[0].length);
      const nameMatch = afterTag.match(/^\s+([a-zA-Z_][a-zA-Z0-9_]*)/);
      if (nameMatch) {
        const nameStart = dt.index + dt[0].length + nameMatch.index! + nameMatch[0].indexOf(nameMatch[1]);
        const sPos = editor.document.positionAt(offset + nameStart);
        const ePos = editor.document.positionAt(offset + nameStart + nameMatch[1].length);
        buckets.docParamName.push({ range: new vscode.Range(sPos, ePos) });
      }
    }
  }
}

export function activate(context: vscode.ExtensionContext): void {
  const hoverProvider = vscode.languages.registerHoverProvider(
    "picoc",
    { provideHover }
  );

  const completionProvider = vscode.languages.registerCompletionItemProvider(
    "picoc",
    { provideCompletionItems },
    "."
  );

  const docTagCompletionProvider = vscode.languages.registerCompletionItemProvider(
    "picoc",
    { provideCompletionItems: provideDocTagCompletions },
    "@"
  );

  const definitionProvider = vscode.languages.registerDefinitionProvider(
    "picoc",
    { provideDefinition }
  );

  // Trigger decoration updates on editor changes
  if (vscode.window.activeTextEditor) {
    updateCommentDecorations(vscode.window.activeTextEditor);
  }
  const onEditorChange = vscode.window.onDidChangeActiveTextEditor((editor) => {
    if (editor) updateCommentDecorations(editor);
  });
  const onDocChange = vscode.workspace.onDidChangeTextDocument((event) => {
    const editor = vscode.window.activeTextEditor;
    if (editor && event.document === editor.document) {
      updateCommentDecorations(editor);
    }
  });

  context.subscriptions.push(
    hoverProvider,
    completionProvider,
    docTagCompletionProvider,
    definitionProvider,
    onEditorChange,
    onDocChange,
    ...Object.values(commentTagDecorations)
  );
}

// --- Hover provider: show function signature + params + doc when available ---

interface FunctionInfo {
  name: string;
  signature: string;
  params: string;
  returnType: string;
  doc: string;
}

function provideHover(
  document: vscode.TextDocument,
  position: vscode.Position
): vscode.ProviderResult<vscode.Hover> {
  const wordRange = document.getWordRangeAtPosition(position);
  if (!wordRange) return null;

  const word = document.getText(wordRange);
  if (!/^[a-zA-Z_][a-zA-Z0-9_]*$/.test(word)) return null;

  const text = document.getText();
  const lines = document.getText().split(/\r?\n/);

  // Find a function declaration or definition whose name matches the word
  // Pattern: name(params) -> returnType  [optional ; or newline then {]
  const funcRegex = new RegExp(
    `\\b(${escapeRegex(word)})\\s*\\(([^)]*)\\)\\s*->\\s*([^;{\\n]+)`,
    "g"
  );

  let match: RegExpExecArray | null;
  const candidates: FunctionInfo[] = [];
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

  // Also look for variable declarations in the current file
  if (candidates.length === 0) {
    const varHoverRegex = new RegExp(
      `\\b(?:export(?:\\s+const)?|let(?:\\s+(?:const|readonly))?|var|global)\\s+(${escapeRegex(word)})\\s*:\\s*([a-zA-Z_][a-zA-Z0-9_ ]*)`,
      "g"
    );
    while ((match = varHoverRegex.exec(text)) !== null) {
      const varType = match[2].trim();
      const modifier = match[0].substring(0, match[0].indexOf(word)).trim();
      const sig = `${modifier} ${word}: ${varType}`;
      const beforeMatch = text.substring(0, match.index);
      const lineIndex = (beforeMatch.match(/\n/g) || []).length;
      const doc = getDocAbove(lines, lineIndex);

      candidates.push({ name: word, signature: sig, params: "", returnType: varType, doc });
    }
  }

  // If no candidates in the current file, check imported files
  if (candidates.length === 0) {
    const importedSymbols = getImportedSymbols(document);
    for (const sym of importedSymbols) {
      if (sym.name === word) {
        if (sym.kind === "function") {
          const paramStr = sym.signature.match(/\(([^)]*)\)/)?.[1] ?? "";
          const retType = sym.signature.match(/->\s*(.+)/)?.[1]?.trim() ?? "void";
          candidates.push({
            name: sym.name,
            signature: sym.signature,
            params: paramStr,
            returnType: retType,
            doc: sym.doc,
          });
        } else {
          candidates.push({
            name: sym.name,
            signature: sym.signature,
            params: "",
            returnType: "",
            doc: sym.doc,
          });
        }
      }
    }
  }

  const found = candidates.length === 0
    ? null
    : candidates.find((c) => c.doc.length > 0) ?? candidates[0];

  if (!found) return null;

  const parts: string[] = [];
  parts.push("```picoc\n" + found.signature + "\n```");

  if (found.doc) {
    const parsed = parseDocTags(found.doc);
    if (parsed.brief) {
      parts.push(parsed.brief);
    }
    if (parsed.params.length > 0) {
      parts.push("**Parameters:**\n" + parsed.params.map(
        (p) => `- \`${p.name}\` — ${p.desc}`
      ).join("\n"));
    } else if (found.params && found.params !== "void") {
      parts.push("**Parameters:** " + found.params);
    }
    if (parsed.returns) {
      parts.push("**Returns:** " + parsed.returns);
    } else {
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
  } else {
    if (found.params && found.params !== "void") {
      parts.push("**Parameters:** " + found.params);
    }
    parts.push("**Returns:** " + found.returnType);
  }

  const content = new vscode.MarkdownString(parts.join("\n\n"));
  content.isTrusted = true;
  return new vscode.Hover(content, wordRange);
}

interface ParsedDoc {
  brief: string;
  params: { name: string; desc: string }[];
  returns: string;
  notes: string[];
  warnings: string[];
  deprecated: string;
  throws: string[];
  see: string[];
  example: string;
  other: string;
}

function parseDocTags(doc: string): ParsedDoc {
  const result: ParsedDoc = {
    brief: "", params: [], returns: "", notes: [], warnings: [],
    deprecated: "", throws: [], see: [], example: "", other: "",
  };

  const lines = doc.split("\n");
  const otherLines: string[] = [];
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
        } else {
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

function escapeRegex(s: string): string {
  return s.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

/** Collect documentation lines immediately above the given line (0-based). */
function getDocAbove(lines: string[], functionLineIndex: number): string {
  const docLines: string[] = [];
  let i = functionLineIndex - 1;
  let inBlockComment = false;

  while (i >= 0) {
    const line = lines[i];
    const trimmed = line.trim();

    // Single-line block comment: /* ... */
    const blockMatch = trimmed.match(/\/\*\s*(.*?)\s*\*\/\s*$/);
    if (blockMatch) {
      docLines.unshift(blockMatch[1].trim());
      i--;
      continue;
    }

    // Line comment: # ...
    if (!inBlockComment && trimmed.startsWith("#")) {
      docLines.unshift(trimmed.slice(1).trim());
      i--;
      continue;
    }

    // Block comment end (scanning upward, so */ is the first thing we hit)
    if (!inBlockComment && trimmed.endsWith("*/")) {
      inBlockComment = true;
      const content = trimmed.slice(0, -2).replace(/^\*\s?/, "").trim();
      if (content) docLines.unshift(content);
      i--;
      continue;
    }

    // Inside a block comment: collect lines until we find /*
    if (inBlockComment) {
      if (trimmed.startsWith("/*")) {
        const content = trimmed.slice(2).replace(/^\*?\s?/, "").trim();
        if (content) docLines.unshift(content);
        break; // found the opening — done
      }
      // Strip leading * (common comment formatting)
      const content = trimmed.replace(/^\*\s?/, "").trim();
      if (content) docLines.unshift(content);
      i--;
      continue;
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

// --- Cross-file symbol resolution ---

interface ImportStatement {
  symbols: string[];  // empty = wildcard (import *)
  filePath: string;   // resolved absolute path
  rawPath: string;    // original path string from import
}

interface ExternalSymbol {
  name: string;
  kind: "function" | "variable";
  signature: string;
  detail: string;
  doc: string;
  filePath: string;
  line: number;
  column: number;
}

function resolveFilePath(rawPath: string, currentFileDir: string): string | null {
  // 1. Try relative to current file
  const relative = path.resolve(currentFileDir, rawPath);
  if (fs.existsSync(relative)) return relative;

  // 2. Try parent directory
  const parent = path.resolve(currentFileDir, "..", rawPath);
  if (fs.existsSync(parent)) return parent;

  // 3. Search workspace folders
  const fileName = path.basename(rawPath);
  const workspaceFolders = vscode.workspace.workspaceFolders;
  if (workspaceFolders) {
    for (const folder of workspaceFolders) {
      const found = findFileRecursive(folder.uri.fsPath, fileName, 4);
      if (found) return found;
    }
  }

  // 4. Search sibling directories of current file
  try {
    const parentDir = path.dirname(currentFileDir);
    const siblings = fs.readdirSync(parentDir, { withFileTypes: true });
    for (const entry of siblings) {
      if (entry.isDirectory()) {
        const candidate = path.join(parentDir, entry.name, fileName);
        if (fs.existsSync(candidate)) return candidate;
      }
    }
  } catch { /* ignore */ }

  return null;
}

function findFileRecursive(dir: string, fileName: string, maxDepth: number): string | null {
  if (maxDepth <= 0) return null;
  try {
    const entries = fs.readdirSync(dir, { withFileTypes: true });
    for (const entry of entries) {
      if (entry.isFile() && entry.name === fileName) {
        return path.join(dir, entry.name);
      }
    }
    for (const entry of entries) {
      if (entry.isDirectory() && !entry.name.startsWith(".") && entry.name !== "node_modules") {
        const found = findFileRecursive(path.join(dir, entry.name), fileName, maxDepth - 1);
        if (found) return found;
      }
    }
  } catch { /* ignore permission errors etc */ }
  return null;
}

function parseImports(document: vscode.TextDocument): ImportStatement[] {
  const text = document.getText();
  const imports: ImportStatement[] = [];
  const dir = path.dirname(document.uri.fsPath);

  // import { sym1, sym2 } from "file.picoc"
  const namedRe = /\bimport\s*\{([^}]+)\}\s*from\s*"([^"]+)"/g;
  let m: RegExpExecArray | null;
  while ((m = namedRe.exec(text)) !== null) {
    const symbols = m[1].split(",").map((s) => s.trim()).filter((s) => s.length > 0);
    const rawPath = m[2];
    const filePath = resolveFilePath(rawPath, dir) ?? path.resolve(dir, rawPath);
    imports.push({ symbols, filePath, rawPath });
  }

  // import * from "file.picoc"
  const wildcardRe = /\bimport\s*\*\s*from\s*"([^"]+)"/g;
  while ((m = wildcardRe.exec(text)) !== null) {
    const rawPath = m[1];
    const filePath = resolveFilePath(rawPath, dir) ?? path.resolve(dir, rawPath);
    imports.push({ symbols: [], filePath, rawPath });
  }

  return imports;
}

const RESERVED_WORDS = new Set([
  "if", "elif", "else", "for", "while", "break", "continue", "return", "pass",
  "and", "or", "not", "xor", "sizeof", "import", "export", "from",
  "let", "var", "const", "readonly", "global", "println",
]);

function extractSymbolsFromFile(filePath: string): ExternalSymbol[] {
  if (!fs.existsSync(filePath)) return [];

  let content: string;
  try {
    content = fs.readFileSync(filePath, "utf-8");
  } catch {
    return [];
  }

  const lines = content.split(/\r?\n/);
  const symbols: ExternalSymbol[] = [];

  // Functions: name(params) -> returnType (skip keywords and calls inside function bodies)
  const funcRegex = /\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\(([^)]*)\)\s*->\s*([^;{\n]+)/g;
  let m: RegExpExecArray | null;
  while ((m = funcRegex.exec(content)) !== null) {
    const name = m[1];
    if (RESERVED_WORDS.has(name)) continue;
    const params = m[2].trim();
    const returnType = m[3].trim().replace(/\s*[;{]\s*$/, "").trim();
    const before = content.substring(0, m.index);
    const lineIndex = (before.match(/\n/g) || []).length;
    const col = m.index - (before.lastIndexOf("\n") + 1);
    const doc = getDocAbove(lines, lineIndex);

    symbols.push({
      name,
      kind: "function",
      signature: `${name}(${params}) -> ${returnType}`,
      detail: `(${params}) -> ${returnType}`,
      doc,
      filePath,
      line: lineIndex,
      column: col,
    });
  }

  // Exported variables: export [const] name: type = ...
  const exportVarRegex = /\bexport\s+(?:const\s+)?([a-zA-Z_][a-zA-Z0-9_]*)\s*:\s*([a-zA-Z_][a-zA-Z0-9_ ]*)/g;
  while ((m = exportVarRegex.exec(content)) !== null) {
    const name = m[1];
    const varType = m[2].trim();
    const before = content.substring(0, m.index);
    const lineIndex = (before.match(/\n/g) || []).length;
    const col = m.index - (before.lastIndexOf("\n") + 1);
    const doc = getDocAbove(lines, lineIndex);

    symbols.push({
      name,
      kind: "variable",
      signature: `export const ${name}: ${varType}`,
      detail: `${varType} (exported constant)`,
      doc,
      filePath,
      line: lineIndex,
      column: col,
    });
  }

  return symbols;
}

interface BuiltinSymbol {
  name: string;
  kind: "function" | "variable";
  signature: string;
  detail: string;
  doc: string;
}

const BUILTIN_LIBRARIES: Record<string, BuiltinSymbol[]> = {
  string: [
    { name: "str_len", kind: "function", signature: "str_len(s: string) -> int", detail: "(s: string) -> int", doc: "Returns the length of the string" },
    { name: "str_cmp", kind: "function", signature: "str_cmp(a: string, b: string) -> int", detail: "(a: string, b: string) -> int", doc: "Lexicographic comparison: returns <0, 0, or >0" },
    { name: "str_concat", kind: "function", signature: "str_concat(a: string, b: string) -> string", detail: "(a: string, b: string) -> string", doc: "Concatenate two strings" },
    { name: "str_from_char", kind: "function", signature: "str_from_char(c: char) -> string", detail: "(c: char) -> string", doc: "Convert a char to a 1-character string" },
    { name: "str_find", kind: "function", signature: "str_find(s: string, ch: char) -> int", detail: "(s: string, ch: char) -> int", doc: "Index of first occurrence of ch, or EOS (-1) if not found" },
    { name: "str_find_first_of", kind: "function", signature: "str_find_first_of(s: string, ch: char) -> int", detail: "(s: string, ch: char) -> int", doc: "Index of first occurrence of ch, or EOS (-1) if not found" },
    { name: "str_find_last_of", kind: "function", signature: "str_find_last_of(s: string, ch: char) -> int", detail: "(s: string, ch: char) -> int", doc: "Index of last occurrence of ch, or EOS (-1) if not found" },
    { name: "str_back", kind: "function", signature: "str_back(s: string) -> char", detail: "(s: string) -> char", doc: "Returns the last character of the string" },
    { name: "str_front", kind: "function", signature: "str_front(s: string) -> char", detail: "(s: string) -> char", doc: "Returns the first character of the string" },
    { name: "substr", kind: "function", signature: "substr(s: string, start: int, length: int) -> string", detail: "(s: string, start: int, length: int) -> string", doc: "Returns a substring starting at 'start' with given length" },
    { name: "substr_step", kind: "function", signature: "substr_step(s: string, start: int, end: int, step: int) -> string", detail: "(s: string, start: int, end: int, step: int) -> string", doc: "Returns a stepped substring from start to end" },
    { name: "char_at", kind: "function", signature: "char_at(s: string, index: int) -> char", detail: "(s: string, index: int) -> char", doc: "Returns the character at the given index" },
    { name: "set_char_at", kind: "function", signature: "set_char_at(s: string, index: int, c: char) -> void", detail: "(s: string, index: int, c: char) -> void", doc: "Sets the character at the given index" },
    { name: "EOS", kind: "variable", signature: "const EOS: int = -1", detail: "int (built-in constant)", doc: "End-of-string sentinel, returned by find functions when not found" },
  ],
  arraylib: [
    { name: "len", kind: "function", signature: "len(arr: int[]) -> int", detail: "(arr: int[]) -> int", doc: "Returns the number of elements in the array" },
    { name: "front", kind: "function", signature: "front(arr: int[]) -> int", detail: "(arr: int[]) -> int", doc: "Returns the first element of the array" },
    { name: "back", kind: "function", signature: "back(arr: int[]) -> int", detail: "(arr: int[]) -> int", doc: "Returns the last element of the array" },
    { name: "find", kind: "function", signature: "find(arr: int[], value: int) -> int", detail: "(arr: int[], value: int) -> int", doc: "Index of first occurrence of value, or -1 if not found" },
    { name: "find_first_of", kind: "function", signature: "find_first_of(arr: int[], value: int) -> int", detail: "(arr: int[], value: int) -> int", doc: "Index of first occurrence of value, or -1 if not found" },
    { name: "find_last_of", kind: "function", signature: "find_last_of(arr: int[], value: int) -> int", detail: "(arr: int[], value: int) -> int", doc: "Index of last occurrence of value, or -1 if not found" },
    { name: "len_str", kind: "function", signature: "len_str(arr: string[]) -> int", detail: "(arr: string[]) -> int", doc: "Returns the number of elements in a string array" },
    { name: "front_str", kind: "function", signature: "front_str(arr: string[]) -> string", detail: "(arr: string[]) -> string", doc: "Returns the first element of a string array" },
    { name: "back_str", kind: "function", signature: "back_str(arr: string[]) -> string", detail: "(arr: string[]) -> string", doc: "Returns the last element of a string array" },
    { name: "find_str", kind: "function", signature: "find_str(arr: string[], value: string) -> int", detail: "(arr: string[], value: string) -> int", doc: "Index of first occurrence in string array, or -1 if not found" },
    { name: "find_first_of_str", kind: "function", signature: "find_first_of_str(arr: string[], value: string) -> int", detail: "(arr: string[], value: string) -> int", doc: "Index of first occurrence in string array, or -1 if not found" },
    { name: "find_last_of_str", kind: "function", signature: "find_last_of_str(arr: string[], value: string) -> int", detail: "(arr: string[], value: string) -> int", doc: "Index of last occurrence in string array, or -1 if not found" },
    { name: "len_double", kind: "function", signature: "len_double(arr: double[]) -> int", detail: "(arr: double[]) -> int", doc: "Returns the number of elements in a double array" },
    { name: "front_double", kind: "function", signature: "front_double(arr: double[]) -> double", detail: "(arr: double[]) -> double", doc: "Returns the first element of a double array" },
    { name: "back_double", kind: "function", signature: "back_double(arr: double[]) -> double", detail: "(arr: double[]) -> double", doc: "Returns the last element of a double array" },
    { name: "find_double", kind: "function", signature: "find_double(arr: double[], value: double) -> int", detail: "(arr: double[], value: double) -> int", doc: "Index of first occurrence in double array, or -1 if not found" },
    { name: "find_first_of_double", kind: "function", signature: "find_first_of_double(arr: double[], value: double) -> int", detail: "(arr: double[], value: double) -> int", doc: "Index of first occurrence in double array, or -1 if not found" },
    { name: "find_last_of_double", kind: "function", signature: "find_last_of_double(arr: double[], value: double) -> int", detail: "(arr: double[], value: double) -> int", doc: "Index of last occurrence in double array, or -1 if not found" },
    { name: "len_char", kind: "function", signature: "len_char(arr: char[]) -> int", detail: "(arr: char[]) -> int", doc: "Returns the number of elements in a char array" },
    { name: "front_char", kind: "function", signature: "front_char(arr: char[]) -> char", detail: "(arr: char[]) -> char", doc: "Returns the first element of a char array" },
    { name: "back_char", kind: "function", signature: "back_char(arr: char[]) -> char", detail: "(arr: char[]) -> char", doc: "Returns the last element of a char array" },
    { name: "find_char", kind: "function", signature: "find_char(arr: char[], value: char) -> int", detail: "(arr: char[], value: char) -> int", doc: "Index of first occurrence in char array, or -1 if not found" },
    { name: "find_first_of_char", kind: "function", signature: "find_first_of_char(arr: char[], value: char) -> int", detail: "(arr: char[], value: char) -> int", doc: "Index of first occurrence in char array, or -1 if not found" },
    { name: "find_last_of_char", kind: "function", signature: "find_last_of_char(arr: char[], value: char) -> int", detail: "(arr: char[], value: char) -> int", doc: "Index of last occurrence in char array, or -1 if not found" },
    { name: "len_bool", kind: "function", signature: "len_bool(arr: bool[]) -> int", detail: "(arr: bool[]) -> int", doc: "Returns the number of elements in a bool array" },
    { name: "front_bool", kind: "function", signature: "front_bool(arr: bool[]) -> bool", detail: "(arr: bool[]) -> bool", doc: "Returns the first element of a bool array" },
    { name: "back_bool", kind: "function", signature: "back_bool(arr: bool[]) -> bool", detail: "(arr: bool[]) -> bool", doc: "Returns the last element of a bool array" },
    { name: "find_bool", kind: "function", signature: "find_bool(arr: bool[], value: bool) -> int", detail: "(arr: bool[], value: bool) -> int", doc: "Index of first occurrence in bool array, or -1 if not found" },
    { name: "find_first_of_bool", kind: "function", signature: "find_first_of_bool(arr: bool[], value: bool) -> int", detail: "(arr: bool[], value: bool) -> int", doc: "Index of first occurrence in bool array, or -1 if not found" },
    { name: "find_last_of_bool", kind: "function", signature: "find_last_of_bool(arr: bool[], value: bool) -> int", detail: "(arr: bool[], value: bool) -> int", doc: "Index of last occurrence in bool array, or -1 if not found" },
    { name: "len_uint", kind: "function", signature: "len_uint(arr: unsigned[]) -> int", detail: "(arr: unsigned[]) -> int", doc: "Returns the number of elements in an unsigned array" },
    { name: "front_uint", kind: "function", signature: "front_uint(arr: unsigned[]) -> unsigned", detail: "(arr: unsigned[]) -> unsigned", doc: "Returns the first element of an unsigned array" },
    { name: "back_uint", kind: "function", signature: "back_uint(arr: unsigned[]) -> unsigned", detail: "(arr: unsigned[]) -> unsigned", doc: "Returns the last element of an unsigned array" },
    { name: "find_uint", kind: "function", signature: "find_uint(arr: unsigned[], value: unsigned) -> int", detail: "(arr: unsigned[], value: unsigned) -> int", doc: "Index of first occurrence in unsigned array, or -1 if not found" },
    { name: "find_first_of_uint", kind: "function", signature: "find_first_of_uint(arr: unsigned[], value: unsigned) -> int", detail: "(arr: unsigned[], value: unsigned) -> int", doc: "Index of first occurrence in unsigned array, or -1 if not found" },
    { name: "find_last_of_uint", kind: "function", signature: "find_last_of_uint(arr: unsigned[], value: unsigned) -> int", detail: "(arr: unsigned[], value: unsigned) -> int", doc: "Index of last occurrence in unsigned array, or -1 if not found" },
  ],
};

function isBuiltinLibrary(rawPath: string): boolean {
  return rawPath in BUILTIN_LIBRARIES;
}

function getBuiltinSymbols(rawPath: string, requestedNames: string[]): ExternalSymbol[] {
  const lib = BUILTIN_LIBRARIES[rawPath];
  if (!lib) return [];

  return lib
    .filter((s) => requestedNames.length === 0 || requestedNames.includes(s.name))
    .map((s) => ({
      name: s.name,
      kind: s.kind,
      signature: s.signature,
      detail: s.detail,
      doc: s.doc,
      filePath: `builtin:${rawPath}`,
      line: 0,
      column: 0,
    }));
}

function getImportedSymbols(document: vscode.TextDocument): ExternalSymbol[] {
  const imports = parseImports(document);
  const result: ExternalSymbol[] = [];
  const seen = new Set<string>();

  for (const imp of imports) {
    if (isBuiltinLibrary(imp.rawPath)) {
      const builtinSyms = getBuiltinSymbols(imp.rawPath, imp.symbols);
      for (const sym of builtinSyms) {
        if (seen.has(sym.name)) continue;
        seen.add(sym.name);
        result.push(sym);
      }
      continue;
    }

    const fileSymbols = extractSymbolsFromFile(imp.filePath);

    for (const sym of fileSymbols) {
      if (seen.has(sym.name)) continue;
      if (imp.symbols.length === 0 || imp.symbols.includes(sym.name)) {
        seen.add(sym.name);
        result.push(sym);
      }
    }
  }

  return result;
}

// --- Auto-completion provider ---

const KEYWORD_COMPLETIONS = [
  "if", "elif", "else", "for", "while", "break", "continue", "return", "pass",
  "and", "or", "not", "xor", "sizeof", "import", "export", "from",
].map((kw) => {
  const item = new vscode.CompletionItem(kw, vscode.CompletionItemKind.Keyword);
  item.detail = "keyword";
  return item;
});

const TYPE_COMPLETIONS = [
  "void", "bool", "int", "double", "unsigned", "char", "string",
].map((t) => {
  const item = new vscode.CompletionItem(t, vscode.CompletionItemKind.TypeParameter);
  item.detail = "type";
  return item;
});

const MODIFIER_COMPLETIONS = [
  "let", "var", "const", "readonly", "global", "export",
].map((m) => {
  const item = new vscode.CompletionItem(m, vscode.CompletionItemKind.Keyword);
  item.detail = "modifier";
  return item;
});

const BUILTIN_COMPLETIONS = [
  { name: "println", sig: "println(value)", kind: "function" },
  { name: "readln", sig: "readln() -> string", kind: "function" },
  { name: "read_int", sig: "read_int() -> int", kind: "function" },
  { name: "read_double", sig: "read_double() -> double", kind: "function" },
  { name: "sizeof", sig: "sizeof(type_or_expr)", kind: "function" },
  { name: "true", sig: "true", kind: "constant" },
  { name: "false", sig: "false", kind: "constant" },
].map((b) => {
  const item = new vscode.CompletionItem(
    b.name,
    b.kind === "constant"
      ? vscode.CompletionItemKind.Constant
      : vscode.CompletionItemKind.Function
  );
  item.detail = "built-in";
  item.documentation = b.sig;
  return item;
});

const ARRAY_LIB_COMPLETIONS = [
  { name: "len", sig: "len(arr: int[]) -> int", doc: "Returns the number of elements in the array" },
  { name: "front", sig: "front(arr: int[]) -> int", doc: "Returns the first element of the array" },
  { name: "back", sig: "back(arr: int[]) -> int", doc: "Returns the last element of the array" },
  { name: "find", sig: "find(arr: int[], value: int) -> int", doc: "Index of first occurrence of value, or -1 if not found" },
  { name: "find_first_of", sig: "find_first_of(arr: int[], value: int) -> int", doc: "Index of first occurrence of value, or -1 if not found" },
  { name: "find_last_of", sig: "find_last_of(arr: int[], value: int) -> int", doc: "Index of last occurrence of value, or -1 if not found" },
  { name: "len_str", sig: "len_str(arr: string[]) -> int", doc: "Number of elements in string array" },
  { name: "front_str", sig: "front_str(arr: string[]) -> string", doc: "First element of string array" },
  { name: "back_str", sig: "back_str(arr: string[]) -> string", doc: "Last element of string array" },
  { name: "find_str", sig: "find_str(arr: string[], value: string) -> int", doc: "Index of first occurrence in string array, or -1" },
  { name: "len_double", sig: "len_double(arr: double[]) -> int", doc: "Number of elements in double array" },
  { name: "front_double", sig: "front_double(arr: double[]) -> double", doc: "First element of double array" },
  { name: "back_double", sig: "back_double(arr: double[]) -> double", doc: "Last element of double array" },
  { name: "find_double", sig: "find_double(arr: double[], value: double) -> int", doc: "Index of first occurrence in double array, or -1" },
  { name: "len_char", sig: "len_char(arr: char[]) -> int", doc: "Number of elements in char array" },
  { name: "front_char", sig: "front_char(arr: char[]) -> char", doc: "First element of char array" },
  { name: "back_char", sig: "back_char(arr: char[]) -> char", doc: "Last element of char array" },
  { name: "find_char", sig: "find_char(arr: char[], value: char) -> int", doc: "Index of first occurrence in char array, or -1" },
  { name: "len_bool", sig: "len_bool(arr: bool[]) -> int", doc: "Number of elements in bool array" },
  { name: "front_bool", sig: "front_bool(arr: bool[]) -> bool", doc: "First element of bool array" },
  { name: "back_bool", sig: "back_bool(arr: bool[]) -> bool", doc: "Last element of bool array" },
  { name: "find_bool", sig: "find_bool(arr: bool[], value: bool) -> int", doc: "Index of first occurrence in bool array, or -1" },
].map((f) => {
  const item = new vscode.CompletionItem(f.name, vscode.CompletionItemKind.Function);
  item.detail = 'import from "arraylib"';
  item.documentation = new vscode.MarkdownString(`${f.doc}\n\n\`${f.sig}\``);
  return item;
});

const STRING_LIB_COMPLETIONS = [
  { name: "str_len", sig: "str_len(s: string) -> int", doc: "Returns the length of the string" },
  { name: "str_cmp", sig: "str_cmp(a: string, b: string) -> int", doc: "Lexicographic comparison: <0, 0, or >0" },
  { name: "str_concat", sig: "str_concat(a: string, b: string) -> string", doc: "Concatenate two strings" },
  { name: "str_from_char", sig: "str_from_char(c: char) -> string", doc: "Convert a char to a 1-character string" },
  { name: "str_find", sig: "str_find(s: string, ch: char) -> int", doc: "Index of first occurrence of ch, or EOS (-1)" },
  { name: "str_find_first_of", sig: "str_find_first_of(s: string, ch: char) -> int", doc: "Index of first occurrence of ch, or EOS (-1)" },
  { name: "str_find_last_of", sig: "str_find_last_of(s: string, ch: char) -> int", doc: "Index of last occurrence of ch, or EOS (-1)" },
  { name: "str_back", sig: "str_back(s: string) -> char", doc: "Last character of the string" },
  { name: "str_front", sig: "str_front(s: string) -> char", doc: "First character of the string" },
  { name: "substr", sig: "substr(s: string, start: int, length: int) -> string", doc: "Substring from start with given length" },
  { name: "substr_step", sig: "substr_step(s: string, start: int, end: int, step: int) -> string", doc: "Stepped substring from start to end" },
  { name: "char_at", sig: "char_at(s: string, index: int) -> char", doc: "Character at index" },
  { name: "set_char_at", sig: "set_char_at(s: string, index: int, c: char) -> void", doc: "Set character at index" },
  { name: "EOS", sig: "const EOS: int = -1", doc: "End-of-string sentinel (not found)" },
].map((f) => {
  const isConst = f.name === "EOS";
  const item = new vscode.CompletionItem(
    f.name,
    isConst ? vscode.CompletionItemKind.Constant : vscode.CompletionItemKind.Function
  );
  item.detail = 'import from "string"';
  item.documentation = new vscode.MarkdownString(`${f.doc}\n\n\`${f.sig}\``);
  return item;
});

function provideCompletionItems(
  document: vscode.TextDocument,
  position: vscode.Position
): vscode.CompletionItem[] {
  const items: vscode.CompletionItem[] = [
    ...KEYWORD_COMPLETIONS,
    ...TYPE_COMPLETIONS,
    ...MODIFIER_COMPLETIONS,
    ...BUILTIN_COMPLETIONS,
    ...STRING_LIB_COMPLETIONS,
    ...ARRAY_LIB_COMPLETIONS,
  ];

  const text = document.getText();

  // Collect function names from definitions and forward declarations
  const funcRegex = /\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\(([^)]*)\)\s*->\s*([^;{\n]+)/g;
  const seenFunctions = new Set<string>();
  let m: RegExpExecArray | null;
  while ((m = funcRegex.exec(text)) !== null) {
    const name = m[1];
    if (seenFunctions.has(name)) continue;
    seenFunctions.add(name);

    const params = m[2].trim();
    const retType = m[3].trim().replace(/\s*[;{]\s*$/, "").trim();
    const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Function);
    item.detail = `(${params}) -> ${retType}`;
    item.insertText = new vscode.SnippetString(`${name}(\${1})`);
    items.push(item);
  }

  // Collect variable names from let/var/global/export declarations
  const varRegex = /\b(?:export(?:\s+const)?|let(?:\s+(?:const|readonly))?|var|global)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*:\s*([a-zA-Z_][a-zA-Z0-9_ ]*)/g;
  const seenVars = new Set<string>();
  while ((m = varRegex.exec(text)) !== null) {
    const name = m[1];
    if (seenVars.has(name)) continue;
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

  // Cross-file symbols from imported files
  const importedSymbols = getImportedSymbols(document);
  for (const sym of importedSymbols) {
    if (sym.kind === "function" && !seenFunctions.has(sym.name)) {
      seenFunctions.add(sym.name);
      const item = new vscode.CompletionItem(sym.name, vscode.CompletionItemKind.Function);
      item.detail = sym.detail + " — " + path.basename(sym.filePath);
      if (sym.doc) {
        item.documentation = new vscode.MarkdownString(sym.doc);
      }
      item.insertText = new vscode.SnippetString(`${sym.name}(\${1})`);
      items.push(item);
    } else if (sym.kind === "variable" && !seenVars.has(sym.name)) {
      seenVars.add(sym.name);
      const item = new vscode.CompletionItem(sym.name, vscode.CompletionItemKind.Variable);
      item.detail = sym.detail + " — " + path.basename(sym.filePath);
      items.push(item);
    }
  }

  return items;
}

// --- Doc-tag completion provider (triggered by @) ---

const DOC_TAG_COMPLETIONS: { label: string; detail: string; snippet: string }[] = [
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

function provideDocTagCompletions(
  document: vscode.TextDocument,
  position: vscode.Position
): vscode.CompletionItem[] | null {
  const lineText = document.lineAt(position).text;
  const prefix = lineText.substring(0, position.character);

  // Only offer @-tag completions inside comments
  const inLineComment = prefix.includes("#");
  const textBefore = document.getText(new vscode.Range(new vscode.Position(0, 0), position));
  const openBlock = textBefore.lastIndexOf("/*");
  const closeBlock = textBefore.lastIndexOf("*/");
  const inBlockComment = openBlock > closeBlock;

  if (!inLineComment && !inBlockComment) return null;

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

function provideDefinition(
  document: vscode.TextDocument,
  position: vscode.Position
): vscode.ProviderResult<vscode.Definition> {
  try {
    const wordRange = document.getWordRangeAtPosition(position);
    if (!wordRange) return null;

    const word = document.getText(wordRange);
    if (!/^[a-zA-Z_][a-zA-Z0-9_]*$/.test(word)) return null;

    const text = document.getText();
    const locations: vscode.Location[] = [];

    // Check if this symbol is imported — if so, go straight to the source file
    const importedSymbols = getImportedSymbols(document);

    for (const sym of importedSymbols) {
      if (sym.name === word) {
        if (sym.filePath.startsWith("builtin:")) continue;
        const uri = vscode.Uri.file(sym.filePath);
        locations.push(new vscode.Location(
          uri,
          new vscode.Position(sym.line, sym.column)
        ));
      }
    }

    if (locations.length > 0) {
      return locations.length === 1 ? locations[0] : locations;
    }

    // Not an imported symbol — search the current file
    let m: RegExpExecArray | null;

    // Function definitions (with body `{`)
    const funcDefRegex = new RegExp(
      `\\b(${escapeRegex(word)})\\s*\\([^)]*\\)\\s*->\\s*[^;{\\n]+\\s*\\{`,
      "g"
    );
    while ((m = funcDefRegex.exec(text)) !== null) {
      const before = text.substring(0, m.index);
      const line = (before.match(/\n/g) || []).length;
      const col = m.index - (before.lastIndexOf("\n") + 1);
      locations.push(new vscode.Location(
        document.uri,
        new vscode.Position(line, col)
      ));
    }

    // Forward declarations (ending in `;`)
    if (locations.length === 0) {
      const funcDeclRegex = new RegExp(
        `\\b(${escapeRegex(word)})\\s*\\([^)]*\\)\\s*->\\s*[^;{\\n]+;`,
        "g"
      );
      while ((m = funcDeclRegex.exec(text)) !== null) {
        const before = text.substring(0, m.index);
        const line = (before.match(/\n/g) || []).length;
        const col = m.index - (before.lastIndexOf("\n") + 1);
        locations.push(new vscode.Location(
          document.uri,
          new vscode.Position(line, col)
        ));
      }
    }

    // Variable declarations: let/var/global/export const <word>
    if (locations.length === 0) {
      const varDeclRegex = new RegExp(
        `\\b(?:export(?:\\s+const)?|let(?:\\s+(?:const|readonly))?|var|global)\\s+(${escapeRegex(word)})\\s*:`,
        "g"
      );
      while ((m = varDeclRegex.exec(text)) !== null) {
        const before = text.substring(0, m.index + m[0].indexOf(word));
        const line = (before.match(/\n/g) || []).length;
        const col = before.length - (before.lastIndexOf("\n") + 1);
        locations.push(new vscode.Location(
          document.uri,
          new vscode.Position(line, col)
        ));
      }
    }

    if (locations.length === 0) return null;
    return locations.length === 1 ? locations[0] : locations;
  } catch {
    return null;
  }
}

export function deactivate(): void {}
