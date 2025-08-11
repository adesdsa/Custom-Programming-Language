#pragma once

#include <string>

class MappedSRC
{
 public:
    MappedSRC(const std::ifstream& fSrc);

    ~MappedSRC() = default;

    char Get();
    std::string Read(size_t ulCharsToRead) noexcept;
    int Seek(size_t ulCharsToSkip) noexcept;
    int Rewind(size_t ulCharsToSkip) noexcept;
    bool IsEOF() const;


 private:
    std::string m_sContent;
    size_t m_ulIndex;
};