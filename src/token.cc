#include "token.h"

#include <string>
#include <vector>

// Since enum names cannot be printed directly, we use this
// workaround.
static std::vector<std::string> token_names{
    "LEFT_PAREN",    "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE", "COMMA",
    "DOT",           "MINUS",       "PLUS",       "SEMICOLON",   "COLON",
    "SLASH",         "STAR",

    "BANG",          "BANG_EQUAL",  "EQUAL",      "EQUAL_EQUAL", "GREATER",
    "GREATER_EQUAL", "LESS",        "LESS_EQUAL",

    "IDENTIFIER",    "STRING",      "NUMBER",

    "AND",           "CLASS",       "ELSE",       "FALSE",       "FN",
    "LAMBDA",        "FOR",         "IF",         "NIL",         "OR",
    "PRINT",         "RETURN",      "SUPER",      "THIS",        "TRUE",
    "VAR",           "WHILE",       "EOF"};

std::string Token::str() const {
  return token_names[type] + " " + lexeme + " " +
         ((type == NUMBER || type == STRING) ? literal : "");
}