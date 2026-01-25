#include "name.h"
#include "type-parse.h"

#include <cassert>

std::string QualifiedName::str() const {
    std::string res = isFullyQualified ? "::" : "";
    assert(tokens.size() == tmplInstantiations.size());
    for (size_t i = 0; i < tokens.size(); ++i) {
        auto const&s = tokens[i];
        res += s.lexeme;

        auto const& tmplInst = tmplInstantiations[i];
        if (tmplInst) {
            res += "< ";
            for (auto const* arg : *tmplInst) {
                res += arg->prettyName();
                if (arg != tmplInst->back()) {
                    res += ", ";
                }
            }
            res += " >";
        }

        if (i != tokens.size() - 1) {
            res += "::";
        }
    }

    return res;
}