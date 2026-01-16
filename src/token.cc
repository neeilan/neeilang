#include "token.h"

#include <string>
#include <vector>

// Since enum names cannot be printed directly, we use this
// workaround.
static std::vector<std::string> token_names{
    "LEFT_PAREN",    "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE", "LEFT_BRACKET",
    "RIGHT_BRACKET", "COMMA",       "DOT",        "MINUS",        "MINUS_MINUS",
    "PLUS",  "PLUS_PLUS", "SEMICOLON",     "COLON",       "SLASH",      "STAR",

    "BANG",          "BANG_EQUAL",  "EQUAL",      "EQUAL_EQUAL", "GREATER",
    "GREATER_EQUAL", "GREATER_GREATER", "LESS",        "LESS_EQUAL", "LESS_LESS",
    "COLON_COLON",   "AMP",  "AMP_EQUAL", "PIPE", "PIPE_EQUAL", "ARROW", "ELLIPSIS",

    "IDENTIFIER",    "STRING",      "NUMBER",

    "AND",           "CLASS",       "ELSE",       "FALSE",       "FN",
    "LAMBDA",        "FOR",         "IF",         "NIL",         "OR",
    "PRINT",         "RETURN",      "SUPER",      "THIS",        "TRUE",
    "VAR",           "WHILE",       "EOF",        "NAMESPACE",   "TEMPLATE",
    "TYPENAME",      "ENUM",        "STATIC",     "INLINE",      "CONST",
    "CONSTEVAL",     "CONSTEXPR",   "EXPLICIT",   "VIRTUAL",      "FRIEND",
    "EXTERN",        "NOEXCEPT",
    "RESERVED_KEYWORD"};

std::string Token::str() const {
  return token_names[type] + " via '" + lexeme + "'" +
         ((type == NUMBER || type == STRING) ? " " + literal : "");
}
