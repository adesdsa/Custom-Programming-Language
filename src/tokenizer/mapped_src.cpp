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

    if(this->m_ulIndex == k_nChunkSize - 1) { // If we are on the last chunk's char.
        GetChunk(true);
    }
    else this->m_ulIndex++;
    
    return true;
}

bool MappedSRC::Rewind() noexcept
{
    if (this->m_uChunkOffset + this->m_ulIndex <= k_nSOF) { return true; }

    if(this->m_ulIndex == 0) { // If we are on the first chunk's char.
        GetChunk(false);
        this->m_ulIndex = k_nChunkSize - 1;
    }
    else this->m_ulIndex--;

    return false;
}

bool MappedSRC::IsEOF() const noexcept
{
    return this->m_ulIndex + this->m_uChunkOffset >= m_uFileSize;
}

bool MappedSRC::GetChunk(bool bOption)
{
    if (this->m_uFileSize == 0) return true; // Incase it's the file is empty.

    if(m_sChunkContent != nullptr) { // If it's the first time we just init don't add.
        if(bOption) this->m_uChunkOffset += k_nChunkSize; // Next chunk.
        else this->m_uChunkOffset -= k_nChunkSize; // Prev chunk.
    }

    size_t chunkLength = static_cast<size_t>(k_nChunkSize) > m_uFileSize - 
    m_uChunkOffset ? m_uFileSize - m_uChunkOffset: 
    static_cast<size_t>(k_nChunkSize); // Getting the min.

    mio::mmap_source temp;
    temp.map(this->m_sFilePath, this->m_uChunkOffset, chunkLength, this->m_ec);

    if (this->m_ec) // Error checking.
    {
        if (this->m_ec.value() == static_cast<int>(std::errc::result_out_of_range)) {
            // Reached end of file
            return true;
        } else throw std::runtime_error("File mapping failed: " + this->m_ec.message());
    }

    m_fSrc.swap(temp);
    this->m_sChunkContent = this->m_fSrc.data();
    this->m_ulIndex = 0;

    return false;
}
