# picoC

## Table of contents

* [Support](#support)
* [Releases](#releases)
* [Compiler options](#compiler-options)
* [Language reference](#language-reference)
  * [Types](#types)
  * [Variables](#variables)
  * [Functions](#functions)
  * [Control flow and operators](#control-flow-and-operators)
  * [Standard libraries](#standard-libraries)
  * [Errors](#errors)
* [Contributing](#contributing)
* [License](#license)

## Support

For bugs or feature requests, please contact us picocdevelopers@gmail.com.

## Releases

Download the latest release from https://gitlab.com/adesdsa/Custom-Programming-Language/-/releases

## Compiler options

```
Usage: picoc [options] <file1.picoc> [file2.picoc ...]
```

| Option            | Description |
|-------------------|-------------|
| `--help`          | Show help and exit |
| `--version`       | Print compiler version (1.0.0) |
| `-o <file>`       | Output executable name (default: `output`) |
| `--out-dir <dir>` | Output directory (default: current directory) |
| `-O0` / `-O1`     | Optimization level (default: 1) |
| `-q`              | Quiet (suppress logs) |
| `-v`              | Verbose logs |
| `--asm`           | Stop after assembly generation (.asm) |
| `--bin`           | Stop after assembler (.obj) |
| `--save-temp`     | Keep intermediate files |
| `--warn-off`      | Disable warnings |
| `--warn-all`      | Enable all warnings |

## Language reference

### Types

| Type     | Description        |
|----------|--------------------|
| `int`    | Signed integer     |
| `unsgined int`   | Unsigned integer   |
| `double` | Double-precision float |
| `bool`   | Boolean (`true` / `false`) |
| `string` | String (mutable)   |
| `char`   | Single character (1-byte, 0–255) |
| `void`   | No value (e.g. function return) |

Arrays are written as `T[]` (e.g. `int[]`, `string[]`). The standard library provides array operations per element type.

### Variables

### Variables Scopes

- **`let`** – define variable at narrowest scope possible.
- **`var`** – define variable at function's scope.
- **`global`** - define in-file global variable.
- **`export`** - define in-file global variable + make it importable by other files.

### Variables Access Right

- **`const`** - defines a compile-time constant. The value must be assigned at declaration and cannot be changed during program execution

- **`readonly`** - defines a runtime constant. readonly variable can be initialized with a value that is only known at runtime

### Template For Variable Decleration

```<scope> (optional) <access-right> (optional) <name>: <type> = <value> (optional)```

### Examples For Variable Decleration

```cpp

x: double = 3.14

let const x: int = 5;

var readonly x: string[] = ["Hello", "World", "!"]

```

### Functions

### Templaes

```<func-name>(params, ...) -> <return-type>```

### Examples

```cpp

add(let a: int, let b: int) -> int
{
    return a + b;
}

```

### Control flow and operators

- **Control flow:** `if` / `else`, `while`, `for`, `break`.
- **Arithmetic:** `+`, `-`, `*`, `/`, `//`, `%`
- **Comparison:** `==`, `!=`, `<`, `<=`, `>`, `>=`
- **Logical:** `and`, `or`, `not`
- **Bitwise:** `and`, `or`, `xor`
- **String:** `+` (concatenation); indexing/slicing: `s[i]`, `s[start:end]`, `s[start:end:step]`

### Standard libraries

Use **`import`** to bring in library names.

| Library    | Example import | Description |
|------------|----------------|-------------|
| **io**     | `import { println, readln, read_int, read_double } from "io"` | Print and read from stdin |
| **string** | `import { str_len, str_cmp, substr, char_at, ... } from "string"` | String functions and `EOS` constant |
| **math**   | `import { sqrt, pow, min, max } from "math"` | Double math |
| **arraylib** | `import { front, back, len, find, ... } from "arraylib"` | Array helpers per element type |


### Errors
| Error Name | Description|
|:----------:|:-----------|
|SyntaxError|Thrown when a character or string incorrectly placed in a command or instruction that causes a failure in execution|
|TypeError  |Thrown when operation or function is applied to an object of inappropriate type |
|NameError  |Thrown when a local or global name is not found|
|ValueError|Thrown when an operation or function receives an argument that has the right type but an inappropriate value|
|LinkingError|Thrown when linker fails to resolve or connect all parts of the code|
|ArgumentError|Thrown when a function is called with incorrect or mismatched arguments|
|AccessRightError|Thrown when code attempts to read or write an object without proper access rights|
|FunctionMissingReturnError|Thrown when not all function's paths have a return statement|
|StatementOutOfBlockError| Thrown when a statement is used outside its proper block, such as `break` or `continue` outside a loop|


## Contributing

Contributions are welcome. Please open an issue or pull request in the project repository.

## License

This project is licensed under the MIT License.
