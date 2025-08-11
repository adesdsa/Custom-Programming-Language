#include "mapped_src.h"

MappedSRC::MappedSRC(std::ifstream &fSrc) : m_ulIndex(0)
{
    // TODO: Change this line to map the src into memory and not read it
    // Reads the entire content of the stream fSrc into the string m_sContent
    this->m_sContent.assign(std::istreambuf_iterator<char>(fSrc), {});
}