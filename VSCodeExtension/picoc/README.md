# PicoC VS Code Extension

Official Visual Studio Code support for the PicoC programming language.

Provides syntax highlighting, language configuration, and integration hooks for the PicoC compiler and language server.

---

## Features

- Syntax highlighting for PicoC source files (`.picoc`)
- Keyword, string, comment, and numeric token coloring
- Bracket matching and auto-closing pairs
- Language configuration for indentation
- Ready for future LSP integration (diagnostics, autocomplete, go-to-definition)

### Example

```picoc
displayAllPrimeNumbersInRange(low: int, high: int) -> void;

main(void) -> int
{
    let low: int = 1;
    let high: int = 100;

    displayAllPrimeNumbersInRange(low, high);
    return 0;
}

# Display all prime numbers in range from low param to high param.
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

        for(let i: int = 2; i < ((low // 2) + 1) ; i = i + 1)
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
    # println(); Display with \n
}
