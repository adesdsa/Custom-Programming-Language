# Change Log

All notable changes to the PicoC extension will be documented in this file.

## [0.3.1] - 2026-03-05

### Added
- **Cross-file Go to Definition** — Ctrl+Click on imported symbols to jump to their definition in the source file
- **Cross-file hover** — hover over imported functions/variables to see their signature and documentation from the source file
- **Cross-file auto-completion** — imported functions and exported variables appear in IntelliSense with source file info
- **Smart file resolution** — finds imported files in parent directories, sibling directories, and workspace folders
- **`import` / `export` / `from` syntax highlighting** — highlighted as keywords with `*` wildcard support
- **Import/export snippets** — `import`, `importall`, and `export` snippet prefixes
- **`export` variable support** — both `export const` and `export` variables are recognized for completions, hover, and Go to Definition

## [0.3.0] - 2026-03-05

### Removed
- Compile command and compile-on-save functionality
- Compiler path and compile-on-save settings

## [0.2.0] - 2026-03-05

### Added
- **IntelliSense auto-completion** for keywords, types, modifiers, built-in functions, and file-local symbols (functions, variables, parameters)
- **Go to Definition** (Ctrl+Click / F12) for functions and variables
- **Code snippets** — 18 snippets for common patterns (`main`, `fn`, `if`, `for`, `while`, `let`, `doc`, and more)
- **Documentation comments** with `@`-tag support (`@brief`, `@param`, `@return`, `@note`, `@warning`, `@deprecated`, etc.)
- **`@`-tag auto-completion** inside comments (triggered by typing `@`)
- **`@`-tag hover rendering** — structured display of parameters, return values, notes, and warnings
- **Enhanced comment markers** — `!`, `?`, `*` with colored highlighting inside comments
- **TODO / FIXME highlighting** with background colors (orange / pink) and scrollbar indicators

### Changed
- Updated extension icon and file icon to peacock logo

## [0.1.0] - 2026-03-05

### Added
- Initial release
- Syntax highlighting for PicoC source files
- Bracket matching and auto-closing pairs
- Line (`#`) and block (`/* */`) comment support
- Compile on save with inline diagnostics
- Hover provider for function signatures and doc comments
