#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "neeilang.h"
#include "scanner.h"
#include "token.h"

bool Neeilang::had_error = false;

void Neeilang::run_file(const char *path)
{
    const std::ifstream file(path);
    std::stringstream src_buffer;

    src_buffer << file.rdbuf();

    run(src_buffer.str());

    if (had_error)
        exit(65); // data format error
}

void Neeilang::run(const std::string &source)
{
    Scanner scanner(source);
    const std::vector<Token> tokens = scanner.scan_tokens();

    std::cout << "Tokens:" << std::endl;
    for (auto token : tokens)
      std::cout << token.str() << std::endl;
}

void Neeilang::error(int line, const std::string &message)
{
    report(line, "", message);
}

void Neeilang::error(Token token, const std::string &message)
{
    if (token.type == END_OF_FILE)
    {
        report(token.line, " at end", message);
    }
    else
    {
        report(token.line, " at '" + token.lexeme + "'", message);
    }
}

// Private

void Neeilang::report(int line,
                 const std::string & occurrence,
                 const std::string & message)
{
    std::cout << "[line " << line << "] Error: "
              << occurrence << " : " << message
              << std::endl;

    had_error = true;
}
