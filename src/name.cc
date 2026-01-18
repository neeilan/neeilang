#include "name.h"
#include "type-parse.h"

std::string QualifiedName::str() const {
    std::string res = isFullyQualified ? "::" : "";
    for (auto const&s : tokens) {
        res += s.lexeme;
        if (&s != &tokens.back()) {
        res += "::";
        }
    }
    if (tmplInstantiation) {
        res += "< ";
        for (auto const* arg : *tmplInstantiation) {
            res += arg->prettyName();
            if (arg != tmplInstantiation->back()) {
                res += ", ";
            }
        }
        res += " >";
    }
    return res;
}