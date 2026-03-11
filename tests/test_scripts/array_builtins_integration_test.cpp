#include <gtest/gtest.h>
#include <cmath>

#include "includes/shared/utils/system_utils.h"
#include "includes/compiler_orchestrator.h"
#include "includes/driver/compiler_options.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

/// Integration test: compile array_builtins_test.picoc and run it, verifying output.
/// Runs on win64, win32, linux64, linux32 when built for the respective platform.
TEST(ArrayBuiltinsIntegrationTest, CompileAndRunArrayTest)
{
    fs::path projectRoot(PROJECT_ROOT);
    fs::path inputFile = projectRoot / "tests" / "inputs" / "array_builtins_test.picoc";
    ASSERT_TRUE(fs::exists(inputFile)) << "Input file not found: " << inputFile.string();

    fs::path outDir = fs::temp_directory_path() / "picoc_array_test";
    fs::create_directories(outDir);
#ifdef _WIN32
    fs::path outExe = outDir / "array_test.exe";
#else
    fs::path outExe = outDir / "array_test";
#endif

    CompilerOptions opts;
    opts.inputFilesPath = { inputFile.wstring() };
    opts.outputFileName = outExe.stem().wstring();
    opts.outputDir = outDir.wstring();
    opts.stopAtBin = false;
    opts.stopAtAsm = false;

    try
    {
        CompilerOrchestrator orchestrator(std::move(opts));
        orchestrator.MainFlow();
    }
    catch (const std::exception& e)
    {
        FAIL() << "Compilation failed: " << e.what();
    }

    ASSERT_TRUE(fs::exists(outExe)) << "Output executable not produced: " << outExe.string();

    std::string output;
    int exitCode = SystemUtils::RunProcess(outExe.string(), output);

    fs::remove_all(outDir);

    ASSERT_EQ(exitCode, 0) << "Program exited with code " << exitCode << "\nOutput:\n" << output;

    std::istringstream iss(output);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(iss, line))
    {
        if (!line.empty())
            lines.push_back(line);
    }

    size_t idx = 0;
    auto expect = [&lines, &idx](const std::string& expected)
    {
        ASSERT_LT(idx, lines.size()) << "Expected \"" << expected << "\" but no more output";
        EXPECT_EQ(lines[idx], expected) << "At output line " << (idx + 1);
        ++idx;
    };
    // Linux32 uses float32 for double; allow small tolerance for float precision
    auto expectFloat = [&lines, &idx](const std::string& expected, double tolerance = 0.001)
    {
        ASSERT_LT(idx, lines.size()) << "Expected \"" << expected << "\" but no more output";
        double expVal = std::stod(expected);
        double actVal = std::stod(lines[idx]);
        EXPECT_NEAR(actVal, expVal, tolerance) << "At output line " << (idx + 1);
        ++idx;
    };

    // 1. a1: int[3] = [1,2,3]
    expect("1");
    expect("2");
    expect("3");

    // 2. a2: int[] = [10,20,30]
    expect("10");
    expect("20");
    expect("30");

    // 3. a3: int[5] = [7,8,9] - last two filled with 0
    expect("7");
    expect("8");
    expect("9");
    expect("0");
    expect("0");

    // 4. b: modification
    expect("100");
    expect("2");
    expect("300");
    expect("4");

    // 5. c: front, back, len
    expect("5");
    expect("20");
    expect("4");

    // 6. d: find, find_first_of, find_last_of
    expect("0");
    expect("0");
    expect("5");
    expect("3");
    expect("-1");

    // 7. e: front_double, back_double, len_double (use expectFloat for Linux32 float precision)
    expectFloat("1.5");
    expectFloat("3.5");
    expect("3");

    // 8. f: find_double family
    expect("0");
    expect("0");
    expect("2");
    expect("3");

    // 9. g: front_char, back_char, len_char (65='A', 68='D') - println_char prints the character
    expect("A");
    expect("D");
    expect("4");

    // 10. h: find_char family
    expect("0");
    expect("0");
    expect("4");
    expect("3");

    // 11. b1: front_bool, back_bool, len_bool
    expect("true");
    expect("true");
    expect("3");

    // 12. b2: find_bool family
    expect("1");
    expect("1");
    expect("3");
    expect("0");

    // 13. dbl: double[5] = [1.0, 2.0] - zero-fill
    expect("1");
    expect("0");
    expect("5");
    expect("1");
    expect("2");
    expect("0");
    expect("0");
    expect("0");

    // 14. m: modify and verify
    expect("11");
    expect("22");
    expect("33");
    expect("11");
    expect("33");
    expect("3");

    EXPECT_EQ(idx, lines.size()) << "Unexpected extra output lines";
}
