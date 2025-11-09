#pragma once

#include <string>
#include <system_error>
#include <stdexcept>
#include <filesystem>
#include <mio/mmap.hpp>

constexpr const int k_nSOF = 0;
constexpr const int k_nChunkSize = 1024;

constexpr const bool k_bNextChunk = true;
constexpr const bool k_bPrevChunk = false;

// Responsible for holding and managing the source file
class MappedSRC
{
public:
    MappedSRC(const std::string& sFilePath);

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
    // @param: bLoadOption: True => next chunck, False => perv chunk.
    // @return: True => EOF, False => otherwise.
    bool GetChunk(bool bLoadOption);


private:
    mio::mmap_source m_mappedChunk;     // The currently memory-mapped portion of the file.
    const char* m_sChunkContent;       // Pointer to the chunk's data inside the mapped region.
    size_t m_ulIndex;                 // Current index (position) within the chunk.
    size_t m_uChunkOffset;           // Offset in the file where this chunk begins.
    std::string m_sFilePath;        // Relative path to the mapped file.  
    size_t m_uFileSize;            // Total size of the file in bytes.
};
