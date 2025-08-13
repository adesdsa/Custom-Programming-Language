#include "lexer.h"

Lexer::Lexer() : m_tokenHandler(), m_tokenFactory(OpenInputFile()) {};