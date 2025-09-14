# Tests

## Why Do We Test?
Testing is a crucial part of software development. It helps ensure:

- **Quality Assurance (QA)** - Tests confirm the software meets defined standards and is ready for publish.

- **Bug Detection** - Constant testing identifies and allows for the correction of bugs before the software is released.

- **Memory Safety Assurance** - Ensure the program does not suffer from memory leaks, buffer overflows, dangling pointers, or undefined behavior.

- **Enhanced Security** - Testing helps to identify and remove security vulnerabilities, protecting against malicious attacks and safeguarding sensitive data. 
---

## Testing Framework: GoogleTest (gtest)
This project uses [GoogleTest](https://github.com/google/googletest) (gtest) as its testing framework.

### Key Concepts
- **Test case / suite** – groups related tests (e.g., `LexerTest`).
- **Test** – checks a specific behavior (e.g., empty input handling).
- **Assertions** – conditions that must hold true:
    - `EXPECT_EQ(a, b)` → checks equality, continues test if it fails.
    - `ASSERT_EQ(a, b)` → checks equality, aborts test if it fails.
    - Few more are: `EXPECT_TRUE`, `EXPECT_THROW`, etc...
- **Fixtures** – reusable setup/teardown for multiple tests.
---

### Test file template 
```cpp

#include <gtest/gtest.h>

#include "relevant_file.h"

TEST(SuiteName, TestName) 
{
    // Test Logic...
}
```

### Example
```cpp
#include <gtest/gtest.h>

#include "tokenizer/lexer.h"

TEST(LexerTest, EmptyInput) 
{
    Lexer lexer("tests/inputs/empty.txt");
    auto tokens = lexer.Tokenize();
    EXPECT_TRUE(tokens.empty());
}
```
---


## CI/CD vs. Manual Testing

### CI/CD (Continuous Integration / Continuous Deployment)
Automated testing that runs as part of the development pipeline. It ensures consistency, prevents regressions, and provides fast feedback on each commit or merge.

### Manual Testing
Human-driven testing where a developer or tester executes the program manually. It is useful for exploratory testing, verifying edge cases, validating real-world scenarios, and checking UI/UX aspects that automated tests may miss.

### Our Testing Choice 
This project is based primarily on **automated testing via GitLab CI/CD**,  
but it supports both approaches:

- On every **push** and **merge request**, the CI/CD pipeline automatically runs tests.

- Manual testing is also supported.  
The project includes built-in tests that can be executed locally:

    ```bash
    cd build
    cmake -DBUILD_TESTS=ON ..
    cmake --build .
    ctest
    ```
---

## Troubleshooting and Support
If you encounter issues when running tests, here are some common problems and solutions:

- **CMake cannot find GoogleTest**
    - Make sure you configured the project with `-DBUILD_TESTS=ON`.
    - If the error persists, check your internet connection since GoogleTest is fetched from GitHub.

- **Unit tests cannot find input or expected files**
    - Ensure that relative paths in the test code point to the correct location.

    > **Note:** The file paths used in CI/CD pipelines are configured relative to the GitLab runner’s working directory.  
    When running tests manually on your local machine, paths may differ. Adjust them as needed, for example, when input files are located outside the build directory. Make sure your changed happen only locally!

- **Tests fail unexpectedly**
    - Run tests with verbose output for more details:
    ```bash
    ctest --verbose
    ```

    - Or run a specific test suite using GoogleTest filters:
    ```bash
    ./build/unit_tests --gtest_filter=LexerTest.*
    ```

- **Build directory issues**
    - If CMake raises errors like `include could not find requested file` during tests, try cleaning the build environment:

    ```bash
    # Remove CMake cache and generated files
    rm -rf CMakeFiles
    rm -f CMakeCache.txt
    rm -rf _deps
    ```

    - After cleaning, you can try building the tests again.