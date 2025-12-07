#pragma once

#include <string>
#include <system_error>
#include <stdexcept>
#include <filesystem>

#include <mio/mmap.hpp>  // mio API

constexpr const int k_nSOF = 0;
constexpr const int k_nChunkSize = 1024;

constexpr const bool k_bNextChunk = true;
constexpr const bool k_bPrevChunk = false;

// Responsible for holding and managing the source file
class MappedSRC
{
public:
    MappedSRC(const std::filesystem::path& filePath);

    ~MappedSRC() = default;

    // Gets current character from the mapped file based on the cursor index.
    char Get() const noexcept;

    // Skip a single character from the mapped file by changing the cursor index.
    // @return: true -> Index is not EOF, false -> Otherwise.
    bool Seek();

    // Go back a single character from the mapped file by changing the stream index.
    // @return: true -> Index is SOF, false -> Otherwise.
    bool Rewind();

    // Check if the cursor index is at EOF.
    // @return: true -> Index is EOF, false -> Otherwise.
    bool IsEOF() const noexcept;

    // The func loads a memory chunk into memory.
    // @param: bLoadOption: true -> next chunck, false -> perv chunk.
    // @return: true -> EOF, false -> otherwise.
    bool GetChunk(bool bLoadOption);

private:
    mio::mmap_source m_mappedChunk;                 // The currently memory-mapped portion of the file.
    const char* m_sChunkContent;                   // Pointer to the chunk's data inside the mapped region.
    size_t m_ulIndex;                             // Current index (position) within the chunk.
    size_t m_uChunkOffset;                       // Offset in the file where this chunk begins.
    const std::filesystem::path& m_filePath;   // Relative path to the mapped file.  
    size_t m_uFileSize;                        // Total size of the file in bytes.
};