#pragma once

#include <string>
#include <boost/iostreams/device/mapped_file.hpp>
#include <string_view>

constexpr int k_nSOF = 0;

// Responsible for holding and managing the source file
class MappedSRC
{
public:
    MappedSRC(boost::iostreams::mapped_file_source&& fSrc);

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
    boost::iostreams::mapped_file_source& m_fSrc;
    const char* m_sContent;
    const size_t m_uSize;
    size_t m_ulIndex;
};
