#include "catch.hpp"
#include "scanner.h"
#include "cxxtype.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using TokenTypes = std::vector<TokenType>;

struct TempFile
{
    explicit TempFile(const char *content)
    {
        tempPath = fs::temp_directory_path() / "test.cxx";
        std::ofstream ofs(tempPath);
        ofs << content;
        ofs.close();
    }

    std::string path() const
    {
        return tempPath.generic_string();
    }

    ~TempFile()
    {
        fs::remove(tempPath);
    }

private:
    fs::path tempPath;
};

TokenTypes tokenTypes(std::vector<Token> const& tok) {
    TokenTypes res; res.reserve(tok.size());
    for (auto const& t : tok) { res.push_back(t.type); }
    return res;
}

TEST_CASE("Simple scan of a type", "[cxxtype]")
{
    TempFile txt{"int const"};
    Scanner scanner{txt.path()};
    auto tokens = scanner.scan_tokens();

    REQUIRE(tokens.size() == 3);
    CHECK(tokenTypes(tokens) == TokenTypes{ IDENTIFIER, CONST, END_OF_FILE});
    CHECK(tokens.front().lexeme == "int");
}

void expectTypeString(const char* src, std::string expected) {
    TempFile txt{src};
    Scanner scanner{txt.path()};
    auto tokens = scanner.scan_tokens();
    CXXTypeParser p(tokens);
    TypePtr ty = p.parseType();
    CHECK(toCanonicalTypeString(ty) == expected);
}

TEST_CASE("Parse various types", "[cxxtype]") {
    expectTypeString("int", "int");
    expectTypeString("int*", "int *");
    expectTypeString("int&", "int &");
    expectTypeString("int&&", "int &&");

    expectTypeString("const int", "int const");
    expectTypeString("int const", "int const");

    expectTypeString("const int*", "int const *");
    expectTypeString("const int *", "int const *");
    expectTypeString("int const*", "int const *");

    expectTypeString("int const&", "int const &");

    expectTypeString("int*const", "int * const");
    expectTypeString("const int*const", "int const * const");
    expectTypeString("int const *const", "int const * const");

    /*
    Other:
    int (*)()
    int (*)(char)
    int (* const)(char)
    int (*)[4]
    int* [3]
    int (*[3])(char)
    decltype(x)*
    auto&&
    */
}
