#include "tokenizer/mapped_src.h"

MappedSRC::MappedSRC(std::ifstream fSrc) : m_ulIndex(0)
{
    // TODO: Change this line to map the src into memory and not read it
    // Reads the entire content of the stream fSrc into the string m_sContent
    this->m_sContent.assign(std::istreambuf_iterator<char>(fSrc), {});
}

char MappedSRC::Get() const noexcept
{
    if (this->IsEOF()) { return '\0'; }

    return this->m_sContent[this->m_ulIndex];
}

bool MappedSRC::Seek() noexcept
{
    if (this->IsEOF()) { return false; }

    this->m_ulIndex++;
    return true;
}

bool MappedSRC::Rewind() noexcept
{
    if (this->m_ulIndex == k_nSOF) { return false; }

    this->m_ulIndex--;
    return true;
}

bool MappedSRC::IsEOF() const noexcept
{
    return this->m_ulIndex == this->m_sContent.size();
}
