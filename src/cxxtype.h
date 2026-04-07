#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <utility>


/* Type specifier
   --------------
   Represents how a variable or function's type is represented *syntactically*
   in it's declaration. This could simply be via the type name (`int x;`),
   or via `auto` or `decltype`.
*/
enum class TypeSpecifierKind {
    DIRECT, AUTO, DECLTYPE
};

enum class TypeKind {
    Primitive,
    Pointer,
    LValueReference,
    RValueReference,
    Array,
    Function,
    Struct,
    TemplateParam,
    Qualified,
    Auto,
    Decltype
};

enum class PrimitiveKind {
    BOOL, FLOAT, DOUBLE, VOID, CHAR, UNSIGNED_CHAR,
    SHORT, UNSIGNED_SHORT, INT, UNSIGNED_INT, LONG,
    UNSIGNED_LONG, LONG_LONG, UNSIGNED_LONG_LONG
};

enum class CVQualifier : unsigned {
    None     = 0,
    Const    = 1 << 0,
    Volatile = 1 << 1
};

inline CVQualifier operator|(CVQualifier a, CVQualifier b) {
    return static_cast<CVQualifier>(
        static_cast<unsigned>(a) | static_cast<unsigned>(b)
    );
}

inline bool hasConst(CVQualifier q) {
    return (static_cast<unsigned>(q) & static_cast<unsigned>(CVQualifier::Const)) != 0;
}

inline bool hasVolatile(CVQualifier q) {
    return (static_cast<unsigned>(q) & static_cast<unsigned>(CVQualifier::Volatile)) != 0;
}

struct CXXType {
    explicit CXXType(TypeKind k) : kind(k) {}
    virtual ~CXXType() = default;
    TypeKind kind;
};

using TypePtr = std::shared_ptr<const CXXType>;

struct PrimitiveType final : public CXXType {
    explicit PrimitiveType(PrimitiveKind pk)
        : CXXType(TypeKind::Primitive), primitiveKind(pk) {}
    PrimitiveKind primitiveKind;
};

struct QualifiedType final : public CXXType {
    QualifiedType(TypePtr base, CVQualifier quals)
        : CXXType(TypeKind::Qualified), baseType(std::move(base)), qualifiers(quals) {}
    TypePtr baseType;
    CVQualifier qualifiers = CVQualifier::None;
};

struct PointerType final : public CXXType {
    PointerType(TypePtr pointee, CVQualifier quals = CVQualifier::None)
        : CXXType(TypeKind::Pointer), pointeeType(std::move(pointee)), qualifiers(quals) {}
    TypePtr pointeeType;
    CVQualifier qualifiers = CVQualifier::None; // e.g. int* const
};

struct LValueReferenceType final : public CXXType {
    explicit LValueReferenceType(TypePtr referred)
        : CXXType(TypeKind::LValueReference), referredType(std::move(referred)) {}
    TypePtr referredType;
};

struct RValueReferenceType final : public CXXType {
    explicit RValueReferenceType(TypePtr referred)
        : CXXType(TypeKind::RValueReference), referredType(std::move(referred)) {}
    TypePtr referredType;
};

struct ArrayType final : public CXXType {
    static constexpr std::size_t UNSIZED = static_cast<std::size_t>(-1);

    ArrayType(TypePtr element, std::size_t count)
        : CXXType(TypeKind::Array), elementType(std::move(element)), elementCount(count) {}

    TypePtr elementType;
    std::size_t elementCount; // UNSIZED means unsized/incomplete array
};

struct CXXFunctionType final : public CXXType {
    CXXFunctionType(TypePtr result, std::vector<TypePtr> params, CVQualifier quals = CVQualifier::None)
        : CXXType(TypeKind::Function),
          resultType(std::move(result)),
          paramTypes(std::move(params)),
          qualifiers(quals) {}

    TypePtr resultType;
    std::vector<TypePtr> paramTypes;

    // For member functions this would matter more; for free functions, usually none.
    CVQualifier qualifiers = CVQualifier::None;
};

enum class Access { Public, Protected, Private };

struct BaseSpec {
    const struct RecordDecl* base;
    Access access = Access::Private;
    bool isVirtual = false;

    std::size_t offsetBytes = 0;
    std::size_t vbptrOffsetBytes = 0;
};

struct FieldDecl {
    std::string name;
    TypePtr type;
    Access access = Access::Private;

    std::size_t offsetBytes = 0;
};

struct MethodDecl {
    std::string name;
    TypePtr functionType;
    Access access = Access::Private;

    bool isVirtual = false;
    bool isOverride = false;
    bool isPure = false;

    int vtableSlot = -1;
};

struct RecordDecl {
    std::string name;

    std::vector<BaseSpec> bases;
    std::vector<FieldDecl> fields;
    std::vector<MethodDecl> methods;

    bool isPolymorphic = false;
    bool isAbstract = false;

    bool isComplete = false;
    std::size_t sizeBytes = 0;
    std::size_t alignBytes = 1;
    bool hasVptr = false;
    std::size_t vptrOffsetBytes = 0;
};

struct StructType final : public CXXType {
    explicit StructType(const RecordDecl* d) : CXXType(TypeKind::Struct), decl(d) {}
    const RecordDecl* decl;
};

struct TemplateParamType final : public CXXType {
    explicit TemplateParamType(std::string n = {})
        : CXXType(TypeKind::TemplateParam), name(std::move(n)) {}

    bool isEngaged() const { return bool(resolvedType); }

    std::string name;
    TypePtr resolvedType;
};

struct AutoType final : public CXXType {
    AutoType() : CXXType(TypeKind::Auto) {}
};

struct DecltypeType final : public CXXType {
    explicit DecltypeType(std::string exprText)
        : CXXType(TypeKind::Decltype), expressionText(std::move(exprText)) {}
    std::string expressionText; // TODO: Make this an Expr*
};

class CXXTypeParser {
public:
    explicit CXXTypeParser(std::vector<Token> toks)
        : tokens_(std::move(toks)) {}

    TypePtr parseType() {
        TypePtr base = parseTypeSpecifier();
        TypeBuilder builder = parseAbstractDeclarator();
        TypePtr result = builder(base);
        validateType(result);
        return result;
    }

    struct ParsedDeclarator {
        std::string name;
        TypePtr type;
    };

    ParsedDeclarator parseNamedType() {
        TypePtr base = parseTypeSpecifier();
        std::string name;
        TypeBuilder builder = parseDeclarator(name, /*allowIdentifier=*/true);
        TypePtr result = builder(base);
        validateType(result);
        return {name, result};
    }

private:
    using TypeBuilder = std::function<TypePtr(TypePtr)>;

    // --------------------------------------------------
    // token helpers
    // --------------------------------------------------

    const Token& peek(std::size_t offset = 0) const {
        static Token eof; eof.type = END_OF_FILE;
        if (pos_ + offset >= tokens_.size()) return eof;
        return tokens_[pos_ + offset];
    }

    bool at(TokenType type) const {
        return peek().type == type;
    }

    bool consume(TokenType type) {
        if (!at(type)) return false;
        ++pos_;
        return true;
    }

    Token expect(TokenType type, const char* msg) {
        if (!at(type)) throw std::runtime_error(msg);
        return tokens_[pos_++];
    }

    bool atIdentifierText(const char* text) const {
        return at(IDENTIFIER) && peek().lexeme == text;
    }

    bool consumeIdentifierText(const char* text) {
        if (!atIdentifierText(text)) return false;
        ++pos_;
        return true;
    }

    Token expectIdentifier(const char* msg) {
        return expect(IDENTIFIER, msg);
    }

    // --------------------------------------------------
    // cv qualifiers
    // --------------------------------------------------

    CVQualifier parseCVQualifiers() {
        CVQualifier q = CVQualifier::None;
        bool progressed = true;

        while (progressed) {
            progressed = false;

            if (consume(CONST)) {
                q = q | CVQualifier::Const;
                progressed = true;
            }

            if (consumeIdentifierText("volatile")) {
                q = q | CVQualifier::Volatile;
                progressed = true;
            }
        }

        return q;
    }

    TypePtr applyQualifiers(TypePtr base, CVQualifier q) {
        if (q == CVQualifier::None) return base;
        return std::make_shared<QualifiedType>(std::move(base), q);
    }

    // --------------------------------------------------
    // type-specifier
    // --------------------------------------------------

    TypePtr parseTypeSpecifier() {
        CVQualifier leadingCV = parseCVQualifiers();

        TypePtr core;

        if (consume(AUTO)) {
            core = std::make_shared<AutoType>();
        } else if (consume(DECLTYPE)) {
            expect(LEFT_PAREN, "expected '(' after decltype");
            std::string exprText = parseDecltypeOperandText();
            expect(RIGHT_PAREN, "expected ')' after decltype operand");
            core = std::make_shared<DecltypeType>(exprText);
        } else {
            core = parseDirectNamedOrBuiltinType();
        }

        CVQualifier trailingCV = parseCVQualifiers();
        return applyQualifiers(core, leadingCV | trailingCV);
    }

    TypePtr parseDirectNamedOrBuiltinType() {
        if (!at(IDENTIFIER)) {
            throw std::runtime_error("expected type specifier");
        }

        if (consumeIdentifierText("bool")) {
            return std::make_shared<PrimitiveType>(PrimitiveKind::BOOL);
        }
        if (consumeIdentifierText("float")) {
            return std::make_shared<PrimitiveType>(PrimitiveKind::FLOAT);
        }
        if (consumeIdentifierText("double")) {
            return std::make_shared<PrimitiveType>(PrimitiveKind::DOUBLE);
        }
        if (consumeIdentifierText("void")) {
            return std::make_shared<PrimitiveType>(PrimitiveKind::VOID);
        }
        if (consumeIdentifierText("char")) {
            return std::make_shared<PrimitiveType>(PrimitiveKind::CHAR);
        }
        if (consumeIdentifierText("short")) {
            consumeIdentifierText("int"); // optional
            return std::make_shared<PrimitiveType>(PrimitiveKind::SHORT);
        }
        if (consumeIdentifierText("int")) {
            return std::make_shared<PrimitiveType>(PrimitiveKind::INT);
        }
        if (consumeIdentifierText("long")) {
            if (consumeIdentifierText("long")) {
                consumeIdentifierText("int"); // optional
                return std::make_shared<PrimitiveType>(PrimitiveKind::LONG_LONG);
            }
            if (consumeIdentifierText("double")) {
                throw std::runtime_error("long double not represented in PrimitiveKind");
            }
            consumeIdentifierText("int"); // optional
            return std::make_shared<PrimitiveType>(PrimitiveKind::LONG);
        }
        if (consumeIdentifierText("unsigned")) {
            return parseUnsignedBuiltinTail();
        }

        // Otherwise this is a user-defined type name or template param.
        Token nameTok = expectIdentifier("expected type name");
        return parseNamedTypeSpecifier(nameTok.lexeme);
    }

    TypePtr parseUnsignedBuiltinTail() {
        if (consumeIdentifierText("char")) {
            return std::make_shared<PrimitiveType>(PrimitiveKind::UNSIGNED_CHAR);
        }
        if (consumeIdentifierText("short")) {
            consumeIdentifierText("int");
            return std::make_shared<PrimitiveType>(PrimitiveKind::UNSIGNED_SHORT);
        }
        if (consumeIdentifierText("long")) {
            if (consumeIdentifierText("long")) {
                consumeIdentifierText("int");
                return std::make_shared<PrimitiveType>(PrimitiveKind::UNSIGNED_LONG_LONG);
            }
            consumeIdentifierText("int");
            return std::make_shared<PrimitiveType>(PrimitiveKind::UNSIGNED_LONG);
        }
        consumeIdentifierText("int");
        return std::make_shared<PrimitiveType>(PrimitiveKind::UNSIGNED_INT);
    }

    TypePtr parseNamedTypeSpecifier(const std::string& name) {
        if (const RecordDecl* rec = lookupRecord(name)) {
            return std::make_shared<StructType>(rec);
        }
        if (lookupTemplateParam(name)) {
            return std::make_shared<TemplateParamType>(name);
        }

        // TODO: If we want unresolved identifiers to still parse as nominal types can
        // replace this with an UnresolvedNamedType node instead of throwing, or
        // have the main parser provide lookup as a service.
        throw std::runtime_error("unknown type name: " + name);
    }

    // --------------------------------------------------
    // decltype payload
    // --------------------------------------------------

    std::string parseDecltypeOperandText() {
        int depth = 0;
        std::string out;
        bool first = true;

        while (true) {
            if (at(END_OF_FILE)) {
                throw std::runtime_error("unterminated decltype operand");
            }

            if (at(RIGHT_PAREN) && depth == 0) {
                break;
            }

            Token t = tokens_[pos_++];

            if (!first) out += " ";
            first = false;
            out += t.lexeme;

            if (t.type == LEFT_PAREN) ++depth;
            else if (t.type == RIGHT_PAREN) --depth;
        }

        return out;
    }

    // --------------------------------------------------
    // declarators
    // --------------------------------------------------

    TypeBuilder parseAbstractDeclarator() {
        std::string ignored;
        return parseDeclarator(ignored, /*allowIdentifier=*/false);
    }

    TypeBuilder parseDeclarator(std::string& outName, bool allowIdentifier) {
        struct PrefixOp {
            enum Kind { Pointer, LRef, RRef } kind;
            CVQualifier quals = CVQualifier::None; // only for Pointer
        };

        std::vector<PrefixOp> prefixOps;

        while (true) {
            if (consume(STAR)) {
                CVQualifier q = parseCVQualifiers(); // int * const
                prefixOps.push_back({PrefixOp::Pointer, q});
                continue;
            }

            if (consume(AMP)) {
                prefixOps.push_back({PrefixOp::LRef, CVQualifier::None});
                continue;
            }

            // AND is &&
            if (at(AND)) {
                consume(AND);
                prefixOps.push_back({PrefixOp::RRef, CVQualifier::None});
                continue;
            }

            break;
        }

        TypeBuilder builder = parseDirectDeclarator(outName, allowIdentifier);

        if (!prefixOps.empty()) {
            TypeBuilder inner = builder;
            builder = [inner, prefixOps](TypePtr base) -> TypePtr {
                TypePtr t = std::move(base);

                for (auto it = prefixOps.rbegin(); it != prefixOps.rend(); ++it) {
                    switch (it->kind) {
                    case PrefixOp::Pointer:
                        t = std::make_shared<PointerType>(t, it->quals);
                        break;
                    case PrefixOp::LRef:
                        t = std::make_shared<LValueReferenceType>(t);
                        break;
                    case PrefixOp::RRef:
                        t = std::make_shared<RValueReferenceType>(t);
                        break;
                    }
                }

                return inner(t);
            };
        }

        return builder;
    }

    TypeBuilder parseDirectDeclarator(std::string& outName, bool allowIdentifier) {
        TypeBuilder builder;

        if (consume(LEFT_PAREN)) {
            builder = parseDeclarator(outName, allowIdentifier);
            expect(RIGHT_PAREN, "expected ')'");
        } else if (allowIdentifier && at(IDENTIFIER)) {
            outName = expectIdentifier("expected identifier").lexeme;
            builder = [](TypePtr base) { return base; };
        } else {
            builder = [](TypePtr base) { return base; };
        }

        for (;;) {
            if (consume(LEFT_PAREN)) {
                std::vector<TypePtr> params;

                if (!consume(RIGHT_PAREN)) {
                    do {
                        params.push_back(parseParameterType());
                    } while (consume(COMMA));

                    expect(RIGHT_PAREN, "expected ')' after parameter list");
                }

                CVQualifier fnCV = parseCVQualifiers();

                TypeBuilder inner = builder;
                builder = [inner, params = std::move(params), fnCV](TypePtr base) -> TypePtr {
                    TypePtr fn = std::make_shared<CXXFunctionType>(base, params, fnCV);
                    return inner(fn);
                };
                continue;
            }

            if (consume(LEFT_BRACKET)) {
                std::size_t count = ArrayType::UNSIZED;

                if (!consume(RIGHT_BRACKET)) {
                    Token n = expect(NUMBER, "expected array bound");
                    count = static_cast<std::size_t>(std::stoull(n.lexeme));
                    expect(RIGHT_BRACKET, "expected ']'");
                }

                TypeBuilder inner = builder;
                builder = [inner, count](TypePtr base) -> TypePtr {
                    TypePtr arr = std::make_shared<ArrayType>(base, count);
                    return inner(arr);
                };
                continue;
            }

            break;
        }

        return builder;
    }

    TypePtr parseParameterType() {
        TypePtr base = parseTypeSpecifier();
        std::string maybeName;
        TypeBuilder builder = parseDeclarator(maybeName, /*allowIdentifier=*/true);
        TypePtr param = builder(base);
        validateType(param);
        return param;
    }

    // --------------------------------------------------
    // validation
    // --------------------------------------------------

    static bool isFunctionType(TypePtr t) {
        return t && t->kind == TypeKind::Function;
    }

    static bool isArrayType(TypePtr t) {
        return t && t->kind == TypeKind::Array;
    }

    static bool isReferenceType(TypePtr t) {
        return t && (t->kind == TypeKind::LValueReference ||
                     t->kind == TypeKind::RValueReference);
    }

    void validateType(TypePtr t) {
        if (!t) return;

        switch (t->kind) {
        case TypeKind::Qualified: {
            auto q = static_cast<const QualifiedType*>(t.get());
            validateType(q->baseType);
            break;
        }

        case TypeKind::Pointer: {
            auto p = static_cast<const PointerType*>(t.get());
            validateType(p->pointeeType);
            break;
        }

        case TypeKind::LValueReference: {
            auto r = static_cast<const LValueReferenceType*>(t.get());
            if (isReferenceType(r->referredType)) {
                throw std::runtime_error("reference to reference is not represented directly");
            }
            validateType(r->referredType);
            break;
        }

        case TypeKind::RValueReference: {
            auto r = static_cast<const RValueReferenceType*>(t.get());
            if (isReferenceType(r->referredType)) {
                throw std::runtime_error("reference to reference is not represented directly");
            }
            validateType(r->referredType);
            break;
        }

        case TypeKind::Array: {
            auto a = static_cast<const ArrayType*>(t.get());
            if (isFunctionType(a->elementType)) {
                throw std::runtime_error("array of functions is not allowed");
            }
            validateType(a->elementType);
            break;
        }

        case TypeKind::Function: {
            auto f = static_cast<const CXXFunctionType*>(t.get());
            if (isFunctionType(f->resultType)) {
                throw std::runtime_error("function returning function is not allowed");
            }
            if (isArrayType(f->resultType)) {
                throw std::runtime_error("function returning array is not allowed");
            }
            validateType(f->resultType);
            for (auto const& p : f->paramTypes) {
                validateType(p);
            }
            break;
        }

        default:
            break;
        }
    }

    // --------------------------------------------------
    // semantic lookup hooks
    // --------------------------------------------------

    const RecordDecl* lookupRecord(const std::string& name) const {
        return nullptr;
    }

    bool lookupTemplateParam(const std::string& name) const {
        return false;
    }

private:
    std::vector<Token> tokens_;
    std::size_t pos_ = 0;
};


struct PrintedTypeParts {
    std::string base;
    std::string declarator;
};

inline std::string primitiveKindToString2(PrimitiveKind k) {
    switch (k) {
    case PrimitiveKind::BOOL:               return "bool";
    case PrimitiveKind::FLOAT:              return "float";
    case PrimitiveKind::DOUBLE:             return "double";
    case PrimitiveKind::VOID:               return "void";
    case PrimitiveKind::CHAR:               return "char";
    case PrimitiveKind::UNSIGNED_CHAR:      return "unsigned char";
    case PrimitiveKind::SHORT:              return "short";
    case PrimitiveKind::UNSIGNED_SHORT:     return "unsigned short";
    case PrimitiveKind::INT:                return "int";
    case PrimitiveKind::UNSIGNED_INT:       return "unsigned int";
    case PrimitiveKind::LONG:               return "long";
    case PrimitiveKind::UNSIGNED_LONG:      return "unsigned long";
    case PrimitiveKind::LONG_LONG:          return "long long";
    case PrimitiveKind::UNSIGNED_LONG_LONG: return "unsigned long long";
    }
    throw std::runtime_error("unknown PrimitiveKind");
}

inline std::string cvSuffix2(CVQualifier q) {
    std::string out;
    if (hasConst(q)) out += " const";
    if (hasVolatile(q)) out += " volatile";
    return out;
}

inline bool needsParensAroundDeclarator(TypePtr t) {
    if (!t) return false;
    return t->kind == TypeKind::Function || t->kind == TypeKind::Array;
}

inline std::string commaJoin(const std::vector<std::string>& parts) {
    std::string out;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i) out += ", ";
        out += parts[i];
    }
    return out;
}

inline PrintedTypeParts splitType(TypePtr t) {
    if (!t) return {"<null>", ""};

    switch (t->kind) {
    case TypeKind::Primitive: {
        auto p = static_cast<const PrimitiveType*>(t.get());
        return {primitiveKindToString2(p->primitiveKind), ""};
    }

    case TypeKind::Struct: {
        auto s = static_cast<const StructType*>(t.get());
        return {s->decl ? s->decl->name : "<anonymous-record>", ""};
    }

    case TypeKind::TemplateParam: {
        auto tp = static_cast<const TemplateParamType*>(t.get());
        return {tp->name.empty() ? "<template-param>" : tp->name, ""};
    }

    case TypeKind::Auto: {
        return {"auto", ""};
    }

    case TypeKind::Decltype: {
        auto d = static_cast<const DecltypeType*>(t.get());
        return {"decltype(" + d->expressionText + ")", ""};
    }

    case TypeKind::Qualified: {
        auto q = static_cast<const QualifiedType*>(t.get());
        auto inner = splitType(q->baseType);
        inner.base += cvSuffix2(q->qualifiers);
        return inner;
    }

    case TypeKind::Pointer: {
        auto p = static_cast<const PointerType*>(t.get());
        auto inner = splitType(p->pointeeType);

        std::string star = "*" + cvSuffix2(p->qualifiers);

        if (!inner.declarator.empty() && needsParensAroundDeclarator(p->pointeeType)) {
            inner.declarator = "(" + star + inner.declarator + ")";
        } else {
            inner.declarator = star + inner.declarator;
        }

        return inner;
    }

    case TypeKind::LValueReference: {
        auto r = static_cast<const LValueReferenceType*>(t.get());
        auto inner = splitType(r->referredType);

        if (!inner.declarator.empty() && needsParensAroundDeclarator(r->referredType)) {
            inner.declarator = "(&" + inner.declarator + ")";
        } else {
            inner.declarator = "&" + inner.declarator;
        }

        return inner;
    }

    case TypeKind::RValueReference: {
        auto r = static_cast<const RValueReferenceType*>(t.get());
        auto inner = splitType(r->referredType);

        if (!inner.declarator.empty() && needsParensAroundDeclarator(r->referredType)) {
            inner.declarator = "(&&" + inner.declarator + ")";
        } else {
            inner.declarator = "&&" + inner.declarator;
        }

        return inner;
    }

    case TypeKind::Array: {
        auto a = static_cast<const ArrayType*>(t.get());
        auto inner = splitType(a->elementType);

        std::string suffix =
            (a->elementCount == ArrayType::UNSIZED)
                ? "[]"
                : "[" + std::to_string(a->elementCount) + "]";

        inner.declarator += suffix;
        return inner;
    }

    case TypeKind::Function: {
        auto f = static_cast<const CXXFunctionType*>(t.get());
        auto inner = splitType(f->resultType);

        std::vector<std::string> params;
        params.reserve(f->paramTypes.size());
        for (auto const& p : f->paramTypes) {
            auto pp = splitType(p);
            params.push_back(pp.base + pp.declarator);
        }

        inner.declarator += "(" + commaJoin(params) + ")";
        inner.declarator += cvSuffix2(f->qualifiers);
        return inner;
    }
    }

    return {"<unknown-type>", ""};
}

inline std::string toCanonicalTypeString(TypePtr t) {
    auto parts = splitType(t);
    if (parts.declarator.empty()) {
        return parts.base;
    }
    return parts.base + " " + parts.declarator;
}

inline std::string toDeclaratorString(TypePtr t, const std::string& name = "") {
    auto parts = splitType(t);
    return parts.base + " " + parts.declarator + name;
}
