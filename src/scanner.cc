#include "scanner.h"
#include "neeilang.h"
#include "token.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

const std::map<std::string, TokenType> Scanner::keywords = {
    {"and", AND},
    {"class", CLASS},
    {"else", ELSE},
    {"false", FALSE},
    {"for", FOR},
    {"fn", FN},
    {"lambda", LAMBDA},
    {"if", IF},
    {"nil", NIL},
    {"or", OR},
    {"print", PRINT},
    {"return", RETURN},
    {"super", SUPER},
    {"this", THIS},
    {"true", TRUE},
    {"var", VAR},
    {"while", WHILE},

    // Reserve C++ keywords
    {"namespace", NAMESPACE},
    {"template", TEMPLATE},
    {"typename", TYPENAME},
    {"struct", CLASS},
    {"enum", ENUM},
    {"static", STATIC},
    {"inline", INLINE},
    {"const", CONST},
    {"consteval", CONSTEVAL},
    {"constexpr", CONSTEXPR},
    {"explicit", EXPLICIT},
    {"virtual", VIRTUAL},
    {"friend", FRIEND},
    {"extern", EXTERN},
    {"noexcept", NOEXCEPT},

    {"alignas", RESERVED_KEYWORD},
    {"alignof", RESERVED_KEYWORD},
    {"auto", RESERVED_KEYWORD},
    // {"bool", RESERVED_KEYWORD},
    {"break", RESERVED_KEYWORD},
    {"continue", RESERVED_KEYWORD},
    {"case", RESERVED_KEYWORD},
    {"catch", RESERVED_KEYWORD},
    {"char", RESERVED_KEYWORD},
    {"concept", RESERVED_KEYWORD},
    {"constinit", RESERVED_KEYWORD},
    {"const_cast", RESERVED_KEYWORD},
    {"co_await", RESERVED_KEYWORD},
    {"co_yield", RESERVED_KEYWORD},
    {"co_return", RESERVED_KEYWORD},
    {"decltype", RESERVED_KEYWORD},
    {"default", RESERVED_KEYWORD},
    {"delete", RESERVED_KEYWORD},
    {"do", RESERVED_KEYWORD},
    {"double", RESERVED_KEYWORD},
    {"dynamic_cast", RESERVED_KEYWORD},
    {"export", RESERVED_KEYWORD},
    // {"float", RESERVED_KEYWORD},
    {"goto", RESERVED_KEYWORD},
    // {"int", RESERVED_KEYWORD},
    {"long", RESERVED_KEYWORD},
    {"mutable", RESERVED_KEYWORD},
    {"new", RESERVED_KEYWORD},
    {"nullptr", RESERVED_KEYWORD},
    {"operator", RESERVED_KEYWORD},
    {"private", RESERVED_KEYWORD},
    {"protected", RESERVED_KEYWORD},
    {"public", RESERVED_KEYWORD},
    {"reinterpret_cast", RESERVED_KEYWORD},
    {"requires", RESERVED_KEYWORD},
    {"short", RESERVED_KEYWORD},
    {"signed", RESERVED_KEYWORD},
    {"sizeof", RESERVED_KEYWORD},
    {"static_assert", RESERVED_KEYWORD},
    {"static_cast", RESERVED_KEYWORD},
    {"switch", RESERVED_KEYWORD},
    {"thread_local", RESERVED_KEYWORD},
    {"throw", RESERVED_KEYWORD},
    {"try", RESERVED_KEYWORD},
    {"typedef", RESERVED_KEYWORD},
    {"typeid", RESERVED_KEYWORD},
    {"union", RESERVED_KEYWORD},
    {"unsigned", RESERVED_KEYWORD},
    {"using", RESERVED_KEYWORD},
    // {"void", RESERVED_KEYWORD},
    {"volatile", RESERVED_KEYWORD},
};

Scanner::Scanner(const std::string &path) {
  add_ctx(std::filesystem::path{path});
}

void Scanner::add_ctx(const std::filesystem::path& path, std::optional<size_t> inclLine) {
  std::ifstream file { path };
  if (!file.is_open()) {
      throw std::runtime_error("Failed to open " + path.string());
  }
  std::vector<const char*> inclPath = ctxs.empty()
    ? std::vector<const char*>{} : ctx().inclPath;
  if (inclLine) {
    fnames.push_back(std::string(inclPath.back()) + ":" +  std::to_string(*inclLine));
    inclPath.pop_back(); inclPath.push_back(fnames.back().c_str());
  }
  fnames.push_back(path.string());
  inclPath.push_back(fnames.back().c_str());

  ctxs.push(SourceCtx{
    .source = std::string(std::istreambuf_iterator<char>(file), {}),
    .path = fnames.back().c_str(),
    .inclPath = inclPath
  });
}


Scanner::SourceCtx& Scanner::ctx() {
  return ctxs.top();
}

std::vector<Token> Scanner::scan_tokens() {
  while (!is_at_end()) {
    // Invariant: All lexemes before current have been scanned
    ctx().start = ctx().current;
    scan_token();
  }

  tokens.push_back(Token(END_OF_FILE, "", "", ctx().line, ctx().inclPath));
  return tokens;
}

bool Scanner::is_at_end() {
  return ctxs.size() == 1 && ctx().current >= ctx().source.length();
}

void Scanner::preprocessor() {
  namespace fs = std::filesystem;

  // Encountered a # - is it at start of line?
  auto curr = ctx().current;
  bool const at_line_start = curr <= 1 || ctx().source[curr-2] == '\n';
  if (!at_line_start) {
    return;
  }

  std::string directive;
  while (is_alphanumeric(peek())) {
    directive += peek();
    advance();
  }
  if (directive == "include") {
    while (peek() == ' ' || peek() == '\t') { advance(); }
    char const opener = advance();
    if (opener != '"' && opener != '<') {
      Neeilang::error(ctx().inclPath, ctx().line, "Malformed #include directive");
    }
    char closer = opener == '<' ? '>' : '"';
    std::string fname;
    while (peek() != closer) {
      fname += advance();
    }
    auto const inclLine = ctx().line;
    advance();
    add_ctx( fs::path{ctx().path}.parent_path() / fname, inclLine );
  } else {
    Neeilang::error(ctx().inclPath, ctx().line, "Unrecognized preprocessor directive " + directive);
  }
}

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
  case '[':
    add_token(LEFT_BRACKET);
    break;
  case ']':
    add_token(RIGHT_BRACKET);
    break;
  case ',':
    add_token(COMMA);
    break;
  case '.':
    add_token(DOT);
    break;
  case '-':
    add_token(match('>') ? ARROW : (match('-') ? MINUS_MINUS : MINUS));
    break;
  case '+':
    add_token(match('+') ? PLUS_PLUS : PLUS);
    break;
  case ';':
    add_token(SEMICOLON);
    break;
  case ':':
    add_token(match(':') ? COLON_COLON : COLON);
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
    add_token(match('=') ? LESS_EQUAL
    : (match('<') ? LESS_LESS : LESS));
    break;
  case '>':
    add_token(match('=') ? GREATER_EQUAL
    : (match('>') ? GREATER_GREATER : GREATER));
    break;
  case '&':
    add_token(match('&') ? AND : (match('=') ? AMP_EQUAL : AMP));
    break; 
  case '|':
    add_token(match('|') ? OR : (match('=') ? PIPE_EQUAL : PIPE));
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
            ctx().line++;
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
    ctx().line++;
    break;
  case '#':
    preprocessor();
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
      Neeilang::error(ctx().inclPath, ctx().line, msg.str());
    }
    break;
  }
}

char Scanner::advance() {
  bool const at_end_of_ctx = ctx().current >= ctx().source.length();
  if (at_end_of_ctx && ctxs.size() > 1) {
    ctxs.pop();
  }
  return ctx().source[ctx().current++];
}

void Scanner::add_token(TokenType type) { add_token(type, ""); }

void Scanner::add_token(TokenType type, std::string literal) {
  std::string text = ctx().source.substr(ctx().start, ctx().current - ctx().start);
  tokens.push_back(Token(type, text, literal, ctx().line, ctx().inclPath));
}

bool Scanner::match(char expected) {
  if (is_at_end() || ctx().source[ctx().current] != expected)
    return false;

  ctx().current++;
  return true;
}

// Lookahead method. Doesn't consume the character.
char Scanner::peek() {
  if (is_at_end())
    return '\0';
  return ctx().source[ctx().current];
}

void Scanner::string() {
  while (peek() != '"' && !is_at_end()) {
    if (peek() == '\n')
      ctx().line++;
    advance();
  }

  // Unterminated string.
  if (is_at_end()) {
    Neeilang::error(ctx().inclPath, ctx().line, "Unterminated string.");
    return;
  }

  // The closing ".
  advance();

  // Trim the surrounding quotes.
  int str_start = ctx().start + 1;
  std::string value = ctx().source.substr(str_start, ctx().current - 1 - str_start);
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

  std::string value = ctx().source.substr(ctx().start, ctx().current - ctx().start);
  add_token(NUMBER, value);
}

char Scanner::peek_next() {
  if (ctx().current + 1 >= ctx().source.length())
    return '\0';
  return ctx().source[ctx().current + 1];
}

void Scanner::identifier() {
  while (is_alphanumeric(peek()))
    advance();

  // See if the identifier is a reserved word.
  std::string text = ctx().source.substr(ctx().start, ctx().current - ctx().start);

  TokenType type = keywords.count(text) ? keywords.at(text) : IDENTIFIER;
  add_token(type);
}

bool Scanner::is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::is_alphanumeric(char c) { return is_alpha(c) || is_digit(c); }