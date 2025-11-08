#pragma once

#include <string>
#include <system_error>
#include <stdexcept>
#include <filesystem>
#include "mio/mmap.hpp"

constexpr int k_nSOF = 0;
constexpr int k_nChunkSize = 1024;

// Responsible for holding and managing the source file
class MappedSRC
{
public:
    MappedSRC(const std::string sFilePath);

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

    // The func loads a memory chunk into memory.
    // @param: bOption: True => next chunck, False => perv chunk.
    // @return: True => EOF, False => otherwise.
    bool GetChunk(bool bOption);


private:
    mio::basic_mmap_source<char, mio::access_mode::read> m_fSrc;
    char* m_sChunkContent;     // Holds the current chunck's content.
    size_t m_ulIndex;           // Holds the current chunck's content index.
    size_t m_uChunkOffset;
    std::error_code m_ec;       // Holds the error code (used by mio).
    std::string m_sFilePath;    
    size_t m_uFileSize;
};
