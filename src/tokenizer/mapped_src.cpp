#include "../../includes/tokenizer/mapped_src.h"

MappedSRC::MappedSRC(boost::iostreams::mapped_file_source&& fSrc) :
m_ulIndex(0), m_fSrc(fSrc), m_uSize(m_fSrc.size())
{
    m_sContent = std::string_view(m_fSrc.data(), m_fSrc.size());
}

char MappedSRC::Get() const noexcept
{
    if (!IsEOF()) return this->m_sContent[m_ulIndex];
    return this->m_sContent[0]; // Shouldn't get to this at first place.
}

bool MappedSRC::Seek() noexcept
{
    if (this->m_ulIndex + 1 >= this->m_uSize) { return false; }

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
    return this->m_ulIndex >= this->m_uSize;
}
