# Platform Features Test - Instructions for Other Platforms

Use this prompt when running the test on **Linux x64**, **Linux x86**, or **Windows x86**:

---

## Prompt for Other Platforms

```
I need you to run the platform features test for the picoC compiler on this system.

1. Build the project first (if not already built):
   - Linux: cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
   - Windows: cmake -S . -B build_test -DCMAKE_BUILD_TYPE=Debug && cmake --build build_test --config Debug

2. Compile the test:
   - Linux: ./build/Custom-Programming-Language -o platform_features_test --out-dir build tests/inputs/platform_features_test.picoc
   - Windows: .\build_test\Debug\Custom-Programming-Language.exe -o platform_features_test --out-dir build_test\Debug tests\inputs\platform_features_test.picoc

3. Run the test with stdin (three lines: hello, 42, 3.14):
   - Linux: printf "hello\n42\n3.14\n" | ./build/platform_features_test
   - Windows PowerShell: "hello`n42`n3.14" | .\build_test\Debug\platform_features_test.exe

4. Report: Exit code should be 0 for PASS. If non-zero, the test failed.
```

---

## What the test covers

- string type, string addition (literals, variables, str_concat)
- string + char addition
- char to string casting (literal and variable)
- substr, str_len, readln, read_int, read_double, char_at

## Expected output (first few lines)

```
hello
hello
hello
world
Abc
Grade: X
a
Z
ell
5
hello
5
42
3.14
h
s
```
