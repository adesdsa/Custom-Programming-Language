# PicoC Language Support

Official Visual Studio Code extension for the **PicoC** programming language.

Provides a full-featured editing experience with syntax highlighting, IntelliSense, compiler integration, and more.

---

## Features

### Syntax Highlighting

Rich, accurate token coloring for all PicoC language constructs — keywords, types, strings, numbers, operators, functions, and variables.

### IntelliSense & Auto-Completion

- **Keywords** — `if`, `while`, `for`, `return`, `and`, `or`, `not`, `xor`, and more
- **Types** — `int`, `bool`, `double`, `void`, `unsigned`
- **Modifiers** — `let`, `var`, `const`, `readonly`, `global`
- **Built-in functions** — `println`, `sizeof`
- **File symbols** — auto-completes function names, variable names, and parameter names from the current file with type information

### Go to Definition

**Ctrl+Click** or **F12** to jump to:
- Function definitions (prefers body over forward declarations)
- Variable declarations (`let`, `var`, `global`)

### Hover Information

Hover over any function to see:
- Full signature
- Parameter list with types
- Return type
- Documentation from `@`-tag comments (see below)

### Code Snippets

Type a prefix and press **Tab** to expand:

| Prefix | Description |
|--------|-------------|
| `main` | Main function entry point |
| `fn` / `func` | Function definition |
| `fndecl` / `declare` | Forward declaration |
| `if` | If statement |
| `ifelse` | If / else |
| `ifelif` | If / elif / else chain |
| `elif` | Elif block |
| `for` | For loop with counter |
| `while` | While loop |
| `let` | Let variable declaration |
| `letconst` | Let const variable |
| `var` | Var variable declaration |
| `global` | Global variable declaration |
| `println` | println call |
| `comment` | Block comment |
| `doc` / `/**` | Documentation comment |
| `docfull` | Full documentation comment |

### Documentation Comments (`@`-tags)

Write structured documentation above your functions using `@`-tags inside comments. Tags are syntax-highlighted (purple for the tag, blue for parameter names) and rendered in hover popups.

Supported tags:

| Tag | Purpose |
|-----|---------|
| `@brief` | Short description of the function |
| `@param name desc` | Document a parameter |
| `@return` / `@returns` | Describe the return value |
| `@note` | Additional notes |
| `@warning` | Usage warnings |
| `@deprecated` | Mark as deprecated |
| `@throws` | Error conditions |
| `@see` | Cross-references |
| `@example` | Usage examples |
| `@author` | Author information |
| `@version` / `@since` | Version information |

Type `@` inside a comment to get auto-complete suggestions for all tags.

**Example:**

```picoc
/*
 * @brief Display all prime numbers in a range
 * @param low The lower bound (inclusive)
 * @param high The upper bound (exclusive)
 * @return void
 */
displayAllPrimeNumbersInRange(low: int, high: int) -> void
{
    ...
}
```

### Enhanced Comment Tags

Special markers inside comments are highlighted with distinct colors:

| Marker | Style | Purpose |
|--------|-------|---------|
| `! text` | Red, bold | Alerts and critical notes |
| `? text` | Blue, italic | Questions and uncertainties |
| `* text` | Green, bold | Highlighted / important info |
| `TODO:` | Orange background | Tasks to complete |
| `FIXME:` | Pink background | Bugs to fix |

`TODO` and `FIXME` also appear as colored markers in the **scrollbar** (orange and pink respectively) so you can spot them at a glance.

### Compiler Integration

- **Compile on save** — automatically compiles the current file when saved (configurable)
- **Compile command** — run `PicoC: Compile Current File` from the command palette
- **Inline diagnostics** — compiler errors and warnings appear as squiggly underlines with messages
- **Proposed fixes** — when the compiler suggests a fix, it's shown alongside the error

### Language Configuration

- Line comments (`#`) and block comments (`/* */`)
- Bracket matching and auto-closing for `{}`, `()`, and `""`
- Surrounding pairs support

---

## Settings

| Setting | Default | Description |
|---------|---------|-------------|
| `picoc.compilerPath` | `"picoc"` | Path to the PicoC compiler executable |
| `picoc.compileOnSave` | `true` | Automatically compile when a `.picoc` file is saved |

---

## Example

```picoc
displayAllPrimeNumbersInRange(low: int, high: int) -> void;

main(void) -> int
{
    let low: int = 1;
    let high: int = 100;

    displayAllPrimeNumbersInRange(low, high);
    return 0;
}

/*
 * @brief Display all prime numbers in range
 * @param low Start of range (inclusive)
 * @param high End of range (exclusive)
 */
displayAllPrimeNumbersInRange(low: int, high: int) -> void
{
    while(low < high)
    {
        let is_prime: bool = true;

        # 0 and 1 are not prime numbers
        if(low < 2)
        {
            is_prime = false;
        }

        for(let i: int = 2; i < ((low // 2) + 1); i = i + 1)
        {
            if(low % i == 0)
            {
                is_prime = false;
                break;
            }
        }

        if(is_prime)
        {
            println(low);
        }

        low = low + 1;
    }
}
```
