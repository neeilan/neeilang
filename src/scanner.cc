#include "scanner.h"
#include "neeilang.h"
#include "token.h"

#include <map>
#include <sstream>
#include <string>
#include <vector>

const std::map<std::string, TokenType> Scanner::keywords = {
    {"and", AND},     {"class", CLASS}, {"else", ELSE},     {"false", FALSE},
    {"for", FOR},     {"fn", FN},       {"lambda", LAMBDA}, {"if", IF},
    {"nil", NIL},     {"or", OR},       {"print", PRINT},   {"return", RETURN},
    {"super", SUPER}, {"this", THIS},   {"true", TRUE},     {"var", VAR},
    {"while", WHILE}};

Scanner::Scanner(const std::string &source) : source(source) {}

std::vector<Token> Scanner::scan_tokens() {
  while (!is_at_end()) {
    // Invariant: All lexemes before current have been scanned
    start = current;
    scan_token();
  }

  tokens.push_back(Token(END_OF_FILE, "", "", line));
  return tokens;
}

bool Scanner::is_at_end() { return current >= source.length(); }
#include <iostream>
void Scanner::scan_token() {
  char c = advance();

  switch (c) {
  case '(':
    add_token(LEFT_PAREN);
    break;
  case ')':
    add_token(RIGHT_PAREN);
    break;
  case '{':
    add_token(LEFT_BRACE);
    break;
  case '}':
    add_token(RIGHT_BRACE);
    break;
  case ',':
    add_token(COMMA);
    break;
  case '.':
    add_token(DOT);
    break;
  case '-':
    add_token(MINUS);
    break;
  case '+':
    add_token(PLUS);
    break;
  case ';':
    add_token(SEMICOLON);
    break;
  case ':':
    add_token(COLON);
    break;
  case '*':
    add_token(STAR);
    break;
  case '!':
    add_token(match('=') ? BANG_EQUAL : BANG);
    break;
  case '=':
    add_token(match('=') ? EQUAL_EQUAL : EQUAL);
    break;
  case '<':
    add_token(match('=') ? LESS_EQUAL : LESS);
    break;
  case '>':
    add_token(match('=') ? GREATER_EQUAL : GREATER);
    break;
  case '/':
    if (match('/')) { // A '//' single-line comment
      while (peek() != '\n' && !is_at_end())
        advance();
    } else if (match('*')) { // A /* multi-line comment
      bool in_comment = true;
      while (in_comment && !is_at_end()) {
        while (!match('*') && !is_at_end()) {
          if (peek() == '\n')
            line++;
          advance();
        }
        // Matched a * - comment ends if we match a /
        in_comment = !match('/');
      }
    } else {
      add_token(SLASH);
    }
    break;
  case ' ':
  case '\r':
  case '\t':
    // Ignore whitespace.
    break;
  case '\n':
    line++;
    break;
  case '"':
    string();
    break; // string literals
  default:
    if (is_digit(c)) {
      number();
    } else if (is_alpha(c)) {
      identifier();
    } else {
      std::ostringstream msg;
      msg << "Unexpected character '" << c << "'";
      Neeilang::error(line, msg.str());
    }
    break;
  }
}

char Scanner::advance() { return source[current++]; }

void Scanner::add_token(TokenType type) { add_token(type, ""); }

void Scanner::add_token(TokenType type, std::string literal) {
  std::string text = source.substr(start, current - start);
  tokens.push_back(Token(type, text, literal, line));
}

bool Scanner::match(char expected) {
  if (is_at_end() || source[current] != expected)
    return false;

  current++;
  return true;
}

// Lookahead method. Doesn't consume the character.
char Scanner::peek() {
  if (is_at_end())
    return '\0';
  return source[current];
}

void Scanner::string() {
  while (peek() != '"' && !is_at_end()) {
    if (peek() == '\n')
      line++;
    advance();
  }

  // Unterminated string.
  if (is_at_end()) {
    Neeilang::error(line, "Unterminated string.");
    return;
  }

  // The closing ".
  advance();

  // Trim the surrounding quotes.
  int str_start = start + 1;
  std::string value = source.substr(str_start, current - 1 - str_start);
  add_token(STRING, value);
}

bool Scanner::is_digit(char c) { return c >= '0' && c <= '9'; }

void Scanner::number() {
  while (is_digit(peek()))
    advance();

  // Look for a fractional part.
  if (peek() == '.' && is_digit(peek_next())) {
    advance(); // Consume the "."
    while (is_digit(peek()))
      advance();
  }

  std::string value = source.substr(start, current - start);
  add_token(NUMBER, value);
}

char Scanner::peek_next() {
  if (current + 1 >= source.length())
    return '\0';
  return source[current + 1];
}

void Scanner::identifier() {
  while (is_alphanumeric(peek()))
    advance();

  // See if the identifier is a reserved word.
  std::string text = source.substr(start, current - start);

  TokenType type = keywords.count(text) ? keywords.at(text) : IDENTIFIER;
  add_token(type);
}

bool Scanner::is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::is_alphanumeric(char c) { return is_alpha(c) || is_digit(c); }