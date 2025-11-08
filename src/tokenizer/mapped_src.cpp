#include "../../includes/tokenizer/mapped_src.h"

MappedSRC::MappedSRC(const std::string sFilePath) :
m_ec(), m_sChunkContent(nullptr), m_ulIndex(0), m_sFilePath(sFilePath), m_uChunkOffset(0), m_uFileSize(std::filesystem::file_size(m_sFilePath))
{
    GetChunk(true);
}

char MappedSRC::Get() const noexcept
{
    if (!IsEOF()) return this->m_sChunkContent[m_ulIndex];
    return '\0'; // Shouldn't get to this at first place.
}

bool MappedSRC::Seek() noexcept
{
    if (this->IsEOF()) { return false; }

    if(this->m_ulIndex == k_nChunkSize - 1) {
        GetChunk(true);
    }
    else this->m_ulIndex++;
    
    return true;
}

bool MappedSRC::Rewind() noexcept
{
    if (this->m_uChunkOffset + this->m_ulIndex <= k_nSOF) { return true; }

    if(this->m_ulIndex == 0) {
        GetChunk(false);
        this->m_ulIndex = k_nChunkSize - 1;
    }
    else this->m_ulIndex--;

    return false;
}

bool MappedSRC::IsEOF() const noexcept
{
    return this->m_ulIndex + this->m_uChunkOffset >= m_uFileSize - 1;
}

bool MappedSRC::GetChunk(bool bOption)
{
    if (this->m_uFileSize == 0) return true;
    if(m_sChunkContent != nullptr) {
        if(bOption) this->m_uChunkOffset += k_nChunkSize;
        else this->m_uChunkOffset -= k_nChunkSize;
    }

    this->m_fSrc = mio::basic_mmap_source<char, mio::access_mode::read>(this->m_sFilePath, this->m_uChunkOffset, k_nChunkSize, this->m_ec);

    if (this->m_ec) 
    {
        if (this->m_ec.value() == static_cast<int>(std::errc::result_out_of_range)) {
            // Reached end of file
            return true;
        } else throw std::runtime_error("File mapping failed: " + this->m_ec.message());
    }

    this->m_sChunkContent = this->m_fSrc.data();
    this->m_ulIndex = 0;

    return false;
}
