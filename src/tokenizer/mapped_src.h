#pragma once

#include <string>

class MappedSRC
{
 public:
    MappedSRC(std::ifstream& fSrc);

    ~MappedSRC() = default;

    // Gets current character from the stream based on the index.
    // @return: Current characters from the stream.
    char Get() const;

    // Read n characters from the stream.
    // @param ulCharsToRead: Number of characters to read from the stream.
    // @return: Current characters from the stream.
    std::string Read(size_t ulCharsToRead) noexcept;

    // Skip a single character from the stream by changing the stream index.
    // @return: True => Index is not EOF, False => Otherwise.
    bool Seek() noexcept;

    // Go back a single character from the stream by changing the stream index.
    // @return: True => Index is SOF, False => Otherwise.
    bool Rewind() noexcept;

    // Check if the stream index is at the end of the file.
    // @return: True => Index is EOF, False => Otherwise.
    bool IsEOF() const;


 private:
    std::string m_sContent;
    size_t m_ulIndex;
};