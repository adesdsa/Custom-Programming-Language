#include "includes/errors/error_models.h"

CompileTimeError::CompileTimeError(
        std::string_view type, 
        std::string content, 
        size_t line, 
        size_t column, 
        std::string hint
    ) :
        m_type(type), 
        m_content(content), 
        m_line(line), 
        m_column(column), 
        m_hint(hint) {}

std::string CompileTimeError::ToString() const noexcept
{
    std::ostringstream oss;

    oss << 
    "at file: " << 
    "(" << this->m_line << "," << this->m_column << "): " <<
    this->m_type << ": " <<
    this->m_content << "\t" <<
    "PROPOSED FIX: " << this->m_hint;

    return oss.str();
}

const char* CompilationAbortException::what() const noexcept
{
    return "Compilation Aborted";
}