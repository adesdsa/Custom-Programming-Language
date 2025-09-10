# Includes

This directory contains all **public header files** of the project.  
They define the **interfaces** (APIs, classes, and constants) that are shared across the project.  
Implementation details should remain in the `src/` directory.

---

## Structure

- `tokenizer/`  
  Contains headers related to lexical analysis (Lexer, Token, TokenFactory, TokenHandler).

- `parser/`  
  (future) Headers for parsing and syntax tree generation.

- `utils/`  
  (future) Shared utility headers such as logging, error handling, etc.

---

## Guidelines

1. **Header Guards - `#pragma once`**  
   - Always protect headers to prevent multiple inclusions.  
   - Use `#pragma once` for simplicity.

2. **Minimal Includes**  
   - Prefer forward declarations instead of including heavy headers when possible.  
   - Keep headers lightweight.

3. **Consistency**  
   - Match the folder structure in `src/`.  
   - For example:  
     - `includes/tokenizer/lexer.h` → `src/tokenizer/lexer.cpp`

4. **Documentation**  
   - Each header should contain a brief description of its purpose.  
   - Document all public classes and methods with comments (e.g., Doxygen style).

---
