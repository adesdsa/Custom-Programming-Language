#pragma once

#include <string>
#include <fstream>

constexpr int k_nSOF = 0;

// Responsible for holding and managing the source file
class MappedSRC
{
public:
    MappedSRC(std::ifstream fSrc);

    ~MappedSRC() = default;

    // Gets current character from the stream based on the index.
    // @return: Current characters from the stream.
    char Get() const noexcept;

    // Skip a single character from the stream by changing the stream index.
    // @return: True => Index is not EOF, False => Otherwise.
    bool Seek() noexcept;

    // Go back a single character from the stream by changing the stream index.
    // @return: True => Index is SOF, False => Otherwise.
    bool Rewind() noexcept;

    // Check if the stream index is at the end of the file.
    // @return: True => Index is EOF, False => Otherwise.
    bool IsEOF() const noexcept;


private:
    std::string m_sContent;
    size_t m_ulIndex;
};