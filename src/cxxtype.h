#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>


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
    Array,
    Function,
    Struct,
    TemplateParam
};


enum class PrimitiveKind {
    BOOL, FLOAT, DOUBLE, VOID, CHAR, UNSIGNED_CHAR,
    SHORT, UNSIGNED_SHORT, INT, UNSIGNED_INT, LONG,
    UNSIGNED_LONG, LONG_LONG, UNSIGNED_LONG_LONG
};

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

struct PointerType final : public CXXType {
    explicit PointerType(TypePtr pointee)
        : CXXType(TypeKind::Pointer), pointeeType(std::move(pointee)) {}

    TypePtr pointeeType; // e.g. int in int*, int* in int**
};

struct ArrayType final : public CXXType {
    static constexpr std::size_t UNSIZED = static_cast<std::size_t>(-1);

    ArrayType(TypePtr element, std::size_t count)
        : CXXType(TypeKind::Array), elementType(std::move(element)), elementCount(count) {}

    TypePtr elementType;        // element type (e.g., int in int[10])
    std::size_t elementCount;   // or UNSIZED
};


// NOTE: C-style variadic functions are not supported
struct CXXFunctionType final : public CXXType {
    CXXFunctionType(TypePtr result, std::vector<TypePtr> params)
        : CXXType(TypeKind::Function),
          resultType(result),
          paramTypes(std::move(params)) {}

    TypePtr resultType;
    std::vector<TypePtr> paramTypes;
};


// Struct type
// -----------

enum class Access { Public, Protected, Private };

struct BaseSpec {
    const struct RecordDecl* base;
    Access access = Access::Private;
    bool isVirtual = false;

    // Layout
    std::size_t offsetBytes = 0;
    std::size_t vbptrOffsetBytes = 0;
};

struct FieldDecl {
    std::string name;
    TypePtr type;
    Access access = Access::Private;

    // Layout
    std::size_t offsetBytes = 0;
};

struct MethodDecl {
    std::string name;
    TypePtr CXXFunctionType;
    Access access = Access::Private;

    bool isVirtual = false;
    bool isOverride = false;
    bool isPure = false;

    // Filled in by vtable building:
    int vtableSlot = -1; // -1 if non-virtual
};

struct RecordDecl {
    std::string name;

    // Inheritance
    std::vector<BaseSpec> bases;

    // Members
    std::vector<FieldDecl> fields;
    std::vector<MethodDecl> methods;

    // Filled in by semantic analysis:
    bool isPolymorphic = false; // has or inherits virtuals
    bool isAbstract = false;    // has pure virtuals unimplemented

    // Layout
    bool isComplete = false;
    std::size_t sizeBytes = 0;
    std::size_t alignBytes = 1;
    bool hasVptr = false;
    std::size_t vptrOffsetBytes = 0;
};

struct StructType final : public CXXType {
    explicit StructType(const RecordDecl* d) : CXXType(TypeKind::Struct /*or Record*/), decl(d) {}
    const RecordDecl* decl;
};

// Represents a `class` or `typename` template parameter.
// Exists in engaged (with a real CXXType bound to it) or
// unengaged state.
struct TemplateParamType final : public CXXType {
    explicit TemplateParamType() : CXXType(TypeKind::TemplateParam) {}

    bool isEngaged() const { return bool(resolvedType); }

    TypePtr resolvedType;
};