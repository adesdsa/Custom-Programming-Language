#include "../../includes/tokenizer/mapped_src.h"

MappedSRC::MappedSRC(const std::string& sFilePath) :
    m_sChunkContent(nullptr),
    m_ulIndex(0),
    m_sFilePath(sFilePath),
    m_uChunkOffset(0),
    m_uFileSize(std::filesystem::file_size(m_sFilePath))
{
    GetChunk(k_bNextChunk);
}

char MappedSRC::Get() const noexcept
{
    return IsEOF() ? '\0' : this->m_sChunkContent[m_ulIndex];
}

bool MappedSRC::Seek() noexcept
{
    if (this->IsEOF()) return false;

    if (this->m_ulIndex == k_nChunkSize - 1)  // if this is last char in buffer
    {
        GetChunk(k_bNextChunk);
    }
    else
    {
        this->m_ulIndex++;
    }
    
    return true;
}

bool MappedSRC::Rewind() noexcept
{
    if (this->m_uChunkOffset + this->m_ulIndex <= k_nSOF) return true;

    if (this->m_ulIndex == 0) // if we are on the start of chunk.
    {
        GetChunk(k_bPrevChunk);
        this->m_ulIndex = k_nChunkSize - 1;
    }
    else 
    {
        this->m_ulIndex--;
    }
    return false;
}

bool MappedSRC::IsEOF() const noexcept
{
    return this->m_ulIndex + this->m_uChunkOffset >= m_uFileSize;
}

bool MappedSRC::GetChunk(bool bLoadOption)
{
    if (this->m_uFileSize == 0) return true;  // incase it's the file is empty

    std::error_code mappingErrorCode;  // holds any mapping error (from mio)

    // for constructor call, don't need to change offset yet
    if (this->m_sChunkContent) 
    {
        if (bLoadOption)  // go to next chunk
        {
            this->m_uChunkOffset += k_nChunkSize;
        }
        else  // go to previous chunk
        {
            this->m_uChunkOffset -= k_nChunkSize;
        }
    }

    // calculate how many bytes remain from the current offset to the end of the file
    size_t remainingBytes = m_uFileSize - m_uChunkOffset;

    // cast chunk size to size_t
    size_t chunkSize = static_cast<size_t>(k_nChunkSize);

    // take the smaller of chunkSize and remainingBytes
    size_t chunkLength = (chunkSize > remainingBytes) ? remainingBytes : chunkSize;

    // map new chunk
    mio::mmap_source temp;
    temp.map(this->m_sFilePath, this->m_uChunkOffset, chunkLength, mappingErrorCode);

    if (mappingErrorCode)  // if error has occur
    {
        if (mappingErrorCode.value() == static_cast<int>(std::errc::result_out_of_range))
        {
            return true;  // reached end of file
        }

        else
        {
            throw std::runtime_error(
                std::string("ERROR: Failed mapping file due to: ") + mappingErrorCode.message()
            );
        }
    }

    // transition from the old chunk to the new chunk
    this->m_mappedChunk.swap(temp);
    this->m_sChunkContent = this->m_mappedChunk.data();
    this->m_ulIndex = 0;

    return false;
}
