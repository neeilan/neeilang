#include <memory>
#include <iostream>
#include <vector>

#include "neeilang.h"
#include "parser.h"
#include "stmt.h"
#include "token.h"
#include "type-parse.h"

Parser::Parser(const std::vector<Token> &tokens)
: tokens(tokens) {}

std::vector<Stmt *> Parser::parse() {
  std::vector<Stmt *> statements;

  while (!at_end()) {
    try {
      statements.push_back(declaration());
    } catch (ParseErr&) {
      synchronize();
    }
  }

  return statements;
}
/*

A declaration can start with:
- type-specifier
- auto
- decltype
- typedef-name
- template-id that names a type
We consider all of these $Type

Note: There is no valid expression that starts with $Type Identifier,
so we use this to eliminate expressions fairly early.

                                Example
                                ------
if identifier [1]               
  VarDecl                       vector<int> | x = {}
  FuncDecl                      vector<int> | x(int)
  Expression                    vector<int> | {5}
else
  Expression (unambiguous)      5 * 5

next, if identifier [1] is $Type:    
  VarDecl                       vector<int> | x(5)
  FuncDecl                      vector<int> | x(int)
  Expression                    vector<int> | {5}
else
  Expression (unambiguous)      vector<int > x() 

next, if next token is an identifier:
  VarDecl                       vector<int> x |(5)
  FuncDecl                      vector<int> x |(int)
else
  Expression (unambiguous)      vector<int> { | 5}

* NOTE: Expression has been eliminated now

next, if next token is '('  
  VarDecl                      vector<int> x ( | 5)
  FuncDecl                     vector<int> x ( |  )
else
  VarDecl (unambiguous)        vector<int> x{};

* NOTE: C++ allows function decls (but not definitions) within a function,
  so we're still in ambiguous territory. We need to parse inside the ().
  
next, if next token is ')'
  FuncDecl                    (language rule)
else
  FuncDecl                    vector<int> x (X x)
  VarDecl                     vector<int> x (X())

next, try parsing as a FuncDecl. If it succeeds, great.
else, parse as a VarDecl.

Vice-versa is not true:
```
// int() is a function type
T x(int());
// type id in parenthesis allowed
T x((T));
i.e. The set of valid expressions overlaps with the set of valid parameter-declarations
but C++ rule is - if you can parse a decl as a function, that wins out.

these should all work:
T x(int);
T x(const int&);
T x(int*);
T x(T&&);
T x(auto&&);
T x(int = 5);
T x(int (*)(int));
T x(int());
```

  */
Stmt *Parser::declaration() {
  Specifiers s = consume_specifiers();

  if (match({NAMESPACE}))
    return namespace_statement();
  if (match({TEMPLATE}))
    return template_statement();
  if (match({CLASS}))
    return class_declaration(s);
  if (match({ENUM}))
    return enum_declaration(s);
  if (match({USING})) {
    return using_declaration();
  } else if (parseStartDecl(/*doCommit*/false)) {    // Variable or function decl
    // This is a func or var decl.
    // We parse it as a func decl if it has:
    // [specifiers] Type Identifier ParameterDeclarationClause
    auto tp = parse_type("var type / func return type");
    if (peek().type == OPERATOR) { //
      return func_statement(tp, std::nullopt, s, "operator overload");
    }
    auto id = consume_qualified_identifier("var / func name");
    auto prevCurrent = current;
    if (match({LEFT_PAREN}) && snoopFnParameterDeclarationClause()){
      current = prevCurrent;
      return func_statement(tp, id, s, "function");
    } else {
      current = prevCurrent;
      return var_declaration(tp, id, s);
    }
  } else {
    return statement();
  }
}

TypeParse Parser::parse_type(const std::string &msg) {
  TypeParse tp;

  if (match({CONST})) {
    tp.isConst = true;
  }

  if (match({DECLTYPE})) {
    // NOTE: decltype is a 'specifier' - not an operator
    consume(LEFT_PAREN, "Expect '(' after decltype");
    tp.declTypeExpr = expression();
    consume(RIGHT_PAREN, "Expect ')' after decltype");
  } else {
    tp.name = consume_qualified_identifier(msg);
  }

  if (match({ELLIPSIS})) {
    tp.isVariadic = true;
  }

  while (match({LEFT_BRACKET})) {
    tp.dims.push_back(expression());
    consume(RIGHT_BRACKET, "Expect matching ']'");
  }

  while (match({STAR})) {
    tp.ptrDepth++;
  }

  if (match({AMP})) {
    tp.isLvalRef = true;
  } else if (match({AND}) && previous().lexeme == "&&") {
      tp.isRvalOrUniversalRef = true;
  }

  return tp;
}

Specifiers Parser::consume_specifiers() {
  Specifiers s{};

  std::vector<TokenType> spToks = {
    STATIC, INLINE, CONST, CONSTEVAL, CONSTEXPR, EXPLICIT,
    VIRTUAL, FRIEND, EXTERN, NOEXCEPT};
  for (auto tok = consume_any_of(spToks); tok; tok = consume_any_of(spToks)) {
    switch (tok->type) {
      case STATIC:
        s.f.isStatic = true;
        break;
      case INLINE:
        s.f.isInline = true;
        break;
      case CONST:
        s.f.isConst = true;
        break;
      case CONSTEXPR:
        s.f.isConstexpr = true;
        break;
      case CONSTEVAL:
        s.f.isConsteval = true;
        break;
      case EXPLICIT:
        s.f.isExplicit = true;
        break;
      case VIRTUAL:
        s.f.isVirtual = true;
        break;
      case FRIEND:
        s.f.isFriend = true;
        break;
      case EXTERN:
        s.f.isExtern = true;
        break;
      case NOEXCEPT:
        s.f.isNoexcept = true;
        break;
      default:
        assert(false && "Unexpected token");
    }
  }

  return s;
}

void Parser::addTemplateName(Token t) {
  templateNames.insert(t.lexeme);
  // possibly-qualified version
  QualifiedName qn = namespaceCtx.qn;
  qn.tokens.push_back(t);
  templateNames.insert(qn.str());

  // fully qualified version
  qn.isFullyQualified = true;
  templateNames.insert(qn.str());
}


Stmt *Parser::var_declaration(TypeParse tp, QualifiedName qn, Specifiers s) {
  Token name = qn.token();
  if (templateCtx.inTemplate) {
    addTemplateName(name);
  } 
  Expr *initializer = nullptr;

/*
    TODO: Handle initialization cases
    https://en.cppreference.com/w/cpp/language/initialization.html
    +------------------+-----------------------------+----------------------------------------+----------------+---------+------------------------------+
    | Syntax kind      | Syntaxes                    | Constructor selection                  | Implicit conv? | Narrows | Notes                        |
    +==================+=============================+========================================+================+=========+==============================+
    | Copy             | T x = expr                  | Implicit constructors only             | Yes            | Yes     | Copy-initialization          |
    +------------------+-----------------------------+----------------------------------------+----------------+---------+------------------------------+
    | Direct           | T x(expr)                   | Implicit + explicit constructors       | Yes            | Yes     | Direct-initialization        |
    +------------------+-----------------------------+----------------------------------------+----------------+---------+------------------------------+
    | *** List-initialization (all forms below) ***                                                                                                     |
    +------------------+-----------------------------+----------------------------------------+----------------+---------+------------------------------+
    | Direct-list      | T x{}                       | (1) initializer_list ctor              | No             | No      | Preferred if exists          |
    |                  |                             | (2) other ctors                        |                |         |                              |
    |                  |                             | (3) aggregate                          |                |         | If aggregate                 |
    |                  |                             | (4) value-init                         |                |         | Only for {}                  |
    +------------------+-----------------------------+----------------------------------------+----------------+---------+------------------------------+
    | Direct-list      | T x{e1, e2}                 | (1) initializer_list ctor              | No             | No      | Preferred if exists          |
    |                  |                             | (2) other ctors                        |                |         |                              |
    |                  |                             | (3) aggregate                          |                |         |                              |
    +------------------+-----------------------------+----------------------------------------+----------------+---------+------------------------------+
    | Copy-list        | T x = {e1, e2}              | (1) initializer_list ctor              | No             | No      | Preferred, explicit ctors    |
    |                  |                             | (2) other ctors (implicit only)        |                |         | forbidden                    |
    |                  |                             | (3) aggregate                          |                |         |                              |
    +------------------+-----------------------------+----------------------------------------+----------------+---------+------------------------------+
    | Designated-list  | T x{ .a = e1, .b = e2 }     | Aggregate (designated)                 | No             | No      | C++20 only                   |
    |                  | T x = { .a = e1 }           |                                        |                |         | Order restricted             |
    +------------------+-----------------------------+----------------------------------------+----------------+---------+------------------------------+
    */
  if (match({EQUAL})) { // TODO: Can be one of several initializations - see above
    initializer = expression();
  } else if (match({LEFT_BRACE})) {
    if (!check(RIGHT_BRACE)) {
      initializer = expression(); // TODO: handle commas / initializer list
    }
    consume(RIGHT_BRACE, "Expect '}' in direct-list initializer"); // TODO
  } else if (match({LEFT_PAREN})) {
    if (!check(RIGHT_PAREN)) {
      initializer = expression(); // TODO: handle commas
    }
    consume(RIGHT_PAREN, "Expect ')' in direct initializer"); // TODO
  }
    
  consume(SEMICOLON, "Expect ';' after variable declaration.");
  tp.isConst = s.f.isConst;
  return new VarStmt(name, tp, initializer);
}

Stmt *Parser::class_declaration(Specifiers) {
  Token name = consume(IDENTIFIER, "Expect class name.");
  if (templateCtx.inTemplate) {
    addTemplateName(name);
  } else {
    typeNames.insert(name.lexeme);
  }


  ClassCtxGuard ccg{classCtx, &name.lexeme};

  Token *superclass = nullptr;

  if (match({LESS})) {
    consume(IDENTIFIER, "Expect superclass name.");
    superclass = new Token(previous());
  }

  consume(LEFT_BRACE, "Expect '{' before class body.");

  std::vector<Token> fields;
  std::vector<TypeParse> field_types;
  std::vector<Stmt *> member_decls;

  while (!check(RIGHT_BRACE) && !at_end()) {
    member_decls.push_back(declaration());
    if (!member_decls.back()->allowedCtxs.classMember) {
    error(previous(), "Invalid class member");
    }
  }

  consume(RIGHT_BRACE, "Expect '}' after class body.");
  consume(SEMICOLON, "Expect ';' after class decl.");

  return new ClassStmt(name, superclass, fields, field_types, member_decls);
}

Stmt *Parser::statement() {
  // We store Tokens for certain statements to enable
  // better error messages.
  if (match({IF}))
    return if_statement(previous());
  if (match({PRINT}))
    return print_statement(previous());
  if (match({RETURN}))
    return return_statement();
  if (match({WHILE}))
    return while_statement(previous());
  if (match({FOR}))
    return for_statement(previous());
  if (match({LEFT_BRACE}))
    return block_statement();
  if (match({STATIC_ASSERT}))
    return static_assert_statement();
  return expression_statement();
}

Stmt *Parser::print_statement(const Token keyword) {
  const Expr *value = expression();
  consume(SEMICOLON, "Expect ';' after value.");
  return new PrintStmt(keyword, value);
}

Stmt *Parser::return_statement() {
  Token keyword = previous();
  Expr *value = nullptr;

  if (!check(SEMICOLON)) {
    value = expression();
  }

  consume(SEMICOLON, "Expect ';' after return value.");
  return new ReturnStmt(keyword, value);
}

Stmt *Parser::block_statement() {
  std::vector<Stmt *> stmts;

  while (!check(RIGHT_BRACE) && !at_end()) {
    stmts.push_back(declaration());
  }

  consume(RIGHT_BRACE, "Expect '}' after block.");

  return new BlockStmt(stmts);
}

Stmt *Parser::using_declaration() {
  bool isNamespace = false, isEnum = false;
  if (match({NAMESPACE})) { isNamespace = true; }
  if (match({ENUM})) { isEnum = true; }

  QualifiedName n1 = consume_qualified_identifier("name for 'using'");
  if (!match({EQUAL})) {

    consume(SEMICOLON, "Expect ';' after using decl.");
    auto * stmt = new UsingStmt(n1);
    stmt->variant.isNamespace = isNamespace;
    stmt->variant.isEnum = isEnum;
    return stmt;
  }

  if (isNamespace || isEnum) {
    error(previous(), "Cannot use 'namespace' or 'enum' in alias statment");
  }
  
  QualifiedName n2 = consume_qualified_identifier("identifier for type alias");
  consume(SEMICOLON, "Expect ';' after type alias decl.");
  return new AliasStmt(n1, n2);
}

// Specifically, scoped enumerators
Stmt *Parser::enum_declaration(Specifiers) {
  std::optional<TypeParse> underlying;
  std::vector<NamedEnumerator> vals;

  consume(CLASS, "Expect 'class' or 'struct' after 'enum'");
  Token name = consume(IDENTIFIER, "Expect name for scoped enum");
  typeNames.insert(name.lexeme);

  if (check({COLON})) {
    consume(COLON, "");
    underlying = parse_type("Expect enum underlying type");
  }

  consume(LEFT_BRACE, "Expect '{' in enum decl");

  if (!check(RIGHT_BRACE)) {

    do {
      NamedEnumerator e;
      e.name = consume(IDENTIFIER, "Expect enumerator name");
      if (check({EQUAL})) {
        consume(EQUAL, "= before enumerator value");
        e.value = consume(NUMBER, "enumerator value"); // not handled, but can be a char literal too
      }
      vals.push_back(e);
    } while (match({COMMA}));

  }

  consume(RIGHT_BRACE, "Expect '}' after enum decl");
  consume(SEMICOLON, "Expect ';' after enum decl.");
  return new ScopedEnum(name.lexeme, vals, underlying);
}

Stmt *Parser::template_statement() {
  // Parse the template typename<...> part (or template <>)
  std::vector<TemplateArg> args;
  consume(LESS, "Expect '<' after 'template'");
  do {
    consume(TYPENAME, "Expect 'typename'");
    bool isVariadic = false;
    if (match({ELLIPSIS})) { isVariadic = true; }
    Token name = consume(IDENTIFIER, "Expect template arg name");
    typeNames.insert(name.lexeme);// TODO: Should be scope based - not a type outside this template
    args.push_back(TemplateArg{.name = name, .isVariadic = isVariadic });
  } while (match({COMMA}));

  consume(GREATER, "Expect '>' after template args");

  TemplateCtxGuard tcg{templateCtx};
  auto* stmt = declaration();
  if (!stmt->allowedCtxs.templatable) {
    error(previous(), "Not a class/function/variable template");
  }
  return new TemplateStmt(args, stmt);
}

Stmt *Parser::static_assert_statement() {
  consume(LEFT_PAREN, "Expect '(' after static_assert");
  auto* expr = expression();
  consume(RIGHT_PAREN, "Unterminated static_assert");
  consume(SEMICOLON, "Expect ';' after static_assert.");
  return new StaticAssertStmt(expr);
}

Stmt *Parser::namespace_statement() {
  std::vector<Stmt *> stmts;

  std::string name;
  if (check(LEFT_BRACE)) {
    // anonymous namespace
  } else {
    QualifiedName qn = consume_qualified_identifier("Expect namespace name");
    name = qn.str();
  }

  NamespaceCtxGuard ncg{namespaceCtx, name};

  consume(LEFT_BRACE, "Expect '{' at start of namespace.");
  while (!check(RIGHT_BRACE) && !at_end()) {
    stmts.push_back(declaration());
  }
  consume(RIGHT_BRACE, "Expect '}' at end of namespace.");

  return new NamespaceStmt(name, stmts);
}

Stmt *Parser::if_statement(Token keyword) {
  consume(LEFT_PAREN, "Expect '(' after 'if'.");
  Expr *condition = expression();
  consume(RIGHT_PAREN, "Expect ')' after if condition.");

  Stmt *then_branch = statement();
  Stmt *else_branch = match({ELSE}) ? statement() : nullptr;

  return new IfStmt(keyword, condition, then_branch, else_branch);
}

Stmt *Parser::while_statement(Token while_tok) {
  consume(LEFT_PAREN, "Expect '(' after 'while'.");
  Expr *condition = expression();
  consume(RIGHT_PAREN, "Expect ')' after condition.");

  consume(LEFT_BRACE, "Expect '{' after while condition.");
  Stmt *body = block_statement();
  return new WhileStmt(while_tok, condition, body);
}

/*
 * Desugar for-loop into equivalent while-loop.
 * for-loop syntax: for (init_stmt? ; condition_expr? ; incr_stmt?) stmt;
 */
Stmt *Parser::for_statement(Token for_tok) {
  consume(LEFT_PAREN, "Expect '(' after 'for'.");

  Stmt *initializer = nullptr;

  if (parseStartDecl(/*doCommit*/false)) {
    auto tp = parse_type("var type in for-loop init");
    auto id = consume_qualified_identifier("var name  in for-loop init");
    initializer = var_declaration(tp, id, {});
  } else {
    initializer = expression_statement();
  }

  Expr *condition = nullptr;

  if (!check(SEMICOLON)) {
    condition = expression();
  }

  consume(SEMICOLON, "Expect ';' after loop condition.");

  Expr *increment = nullptr;
  if (!check(RIGHT_PAREN)) {
    increment = expression();
  }
  const Token &rparen = consume(RIGHT_PAREN, "Expect ')' after for clauses.");

  Stmt *body = statement();

  // Construct block stmt with initializer + desugared while-loop
  if (increment) {
    body = new BlockStmt({body, new ExprStmt(increment, rparen)});
  }

  if (!condition) {
    condition = new BoolLiteral(true);
  }

  body = new WhileStmt(for_tok, condition, body);

  if (initializer) {
    body = new BlockStmt({initializer, body});
  }

  return body;
}

Stmt *Parser::func_statement(TypeParse return_type, std::optional<QualifiedName> qn, Specifiers specifiers, std::string kind) {
  std::optional<Token> name;
  std::optional<TokenType> operatorOverload;

  if (match({OPERATOR})) {
    name = previous();
    if (match({LEFT_PAREN})) {
      if (match({RIGHT_PAREN})) {
        operatorOverload = LEFT_PAREN; // Stand-in for call
      }
    } else if (match({LEFT_BRACKET})) {
      if (match({RIGHT_BRACKET})) {
        operatorOverload = LEFT_BRACKET; // Stand-in for subscript
      }
    } else if (match({
      PLUS, MINUS, BANG, TILDE, PLUS_PLUS, MINUS_MINUS, STAR, SLASH, MOD,
      PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, MOD_EQUAL,
      EQUAL_EQUAL, BANG_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
      AND, OR, // TODO: Check tokens are &&, ||
      AMP, LESS_LESS, GREATER_GREATER, CARET, PIPE, // Bitwise
      COMMA, ARROW, EQUAL, NEW, DELETE
    })) {
      operatorOverload = previous().type;
    } else {
      // TODO: Operators new[], delete[]
    }

    if (!operatorOverload) {
      throw error(peek(), "Not an overloadable operator");
    }
  } else {
    assert(qn);
    name = qn->token();
    if (templateCtx.inTemplate) {
      addTemplateName(*name);
    }
  }

  consume(LEFT_PAREN, "Expect '(' after " + kind + " name.");

  std::vector<Token> parameters;
  std::vector<TypeParse> parameter_types;
  std::map<size_t, Expr*> default_args;
  int argIdx = 0;

  if (!check(RIGHT_PAREN)) {
    do {
      if (parameters.size() >= 8) {
        error(peek(), "Cannot have more than 8 parameters.");
      }

      parameter_types.push_back(parse_type("Expect parameter"));
      if (!check(COMMA) && !check(RIGHT_PAREN)
        && !check(EQUAL) // T x(int = 5); is legal
      ) {
        parameters.push_back(consume(IDENTIFIER, "Expect parameter name."));
      } else {
        parameters.push_back(Token(IDENTIFIER, "$", "", -1, {"<Unknown file>"}));
      }
      if (match({EQUAL})) {
        default_args[argIdx] = expression();
      }
      argIdx++;
    } while (match({COMMA}));
  }

  consume(RIGHT_PAREN, "Expect ')' after parameters.");


  std::vector<Stmt *> body;
  if (!check(SEMICOLON)) {
    consume(LEFT_BRACE, "Expect '{' before " + kind + " body.");
    body.push_back(block_statement());
  } else {
    // TODO: Mark functsmt as declaration only
    consume(SEMICOLON, "");
  }

  auto * func = new FuncStmt(*name, parameters, parameter_types, return_type, body);
  func->setSpecifiers(specifiers);
  func->setOperatorOverload(operatorOverload);
  func->defaultArgs = std::move(default_args);
  return func;
}

Stmt *Parser::expression_statement() {
  Expr *value = expression();
  Token &sc = consume(SEMICOLON, "Expect ';' after expression.");
  return new ExprStmt(value, sc);
}

Expr *Parser::expression() { return assignment(); }

Expr *Parser::assignment() {
  // Don't have a lot of lookahead, so use a 'trick' here:
  // All assignment targets are valid exprs (ex - 'a.prop.b')
  // so parse LHS as an Expr and check that it's an l-value.
  Expr *expr = logical_or();

  auto compoundAssignmentTransform = [](Token const& cmpd, Expr* l, Expr* r) -> Expr* {
#define LOGL(t, o) case t: return new Logical(*l, cmpd.transform(o), *r)
#define BIN(t, o)  case t: return new Binary (*l, cmpd.transform(o), *r)
    switch (cmpd.type) {
      LOGL(AMP_EQUAL,   AND);
      LOGL(PIPE_EQUAL,  OR);
      BIN (PLUS_EQUAL,  PLUS);
      BIN (MINUS_EQUAL, MINUS);
      BIN (SLASH_EQUAL, SLASH);
      BIN (STAR_EQUAL,  STAR);
      BIN (MOD_EQUAL,  MOD);
      default:
        assert(false);
        return nullptr;
    }
#undef LOGL
#undef BIN
  };

  if (match(
    {EQUAL, AMP_EQUAL, PIPE_EQUAL, PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, MOD_EQUAL})) {
    Token equalLike = previous();
    Expr *value = assignment(); // right-associative, so recurse
    if (expr->lvalue()) {
      Variable *variable = dynamic_cast<Variable *>(expr);
      if (equalLike.type == EQUAL) {
        return new Assignment(variable->name, *value);
      } else {
        return new Assignment(variable->name,
          *compoundAssignmentTransform(equalLike, expr, value));
      }
    } else if (expr->is_object_field()) {
      Get *get = static_cast<Get *>(expr);
      if (equalLike.type == EQUAL) {
        return new Set(get->callee, get->name, *value);
      } else {
        return new Set(get->callee, get->name,
          *compoundAssignmentTransform(equalLike, expr, value));
      }
    } else if (expr->is_indexed()) {
      GetIndex *get = static_cast<GetIndex *>(expr);
      if (equalLike.type == EQUAL) {
        return new SetIndex(get->callee, get->bracket, get->index, *value);
      } else {
        return new SetIndex(get->callee, get->bracket, get->index, 
          *compoundAssignmentTransform(equalLike, expr, value));
      }
    }
    error(equalLike, "Invalid assignment target.");
  } else if (match({LESS_LESS, GREATER_GREATER})) {
    Token op = previous();
    Expr *right = assignment(); // right-associative, so recurse
    return new Binary(*expr, op, *right);
  }

  // If no assignment found, fall through to
  // the higher-precedence, valid Expr.
  return expr;
}

Expr *Parser::logical_or() {
  Expr *expr = logical_and();

  while (match({OR})) {
    Token op = previous();
    Expr *right = logical_and();
    expr = new Logical(*expr, op, *right);
  }

  return expr;
}

Expr *Parser::logical_and() {
  Expr *expr = bitwise_or();

  while (match({AND})) {
    Token op = previous();
    Expr *right = bitwise_or();
    expr = new Logical(*expr, op, *right);
  }

  return expr;
}

Expr *Parser::bitwise_or() {
  Expr *expr = bitwise_and();

  while (match({PIPE})) {
    Token op = previous();
    Expr *right = bitwise_and();
    expr = new Binary(*expr, op, *right);
  }
  return expr;
}

Expr *Parser::bitwise_and() {
  Expr *expr = equality();

  while (match({AMP})) {
    Token op = previous();
    Expr *right = equality();
    expr = new Binary(*expr, op, *right);
  }
  return expr;
}

Expr *Parser::equality() {
  Expr *expr = comparison();

  while (match({BANG_EQUAL, EQUAL_EQUAL})) {
    Token op = previous();
    Expr *right = comparison();
    expr = (new Binary(*expr, op, *right));
  }

  return expr;
}

bool Parser::match(const std::vector<TokenType> &types) {
  for (const TokenType &type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

std::optional<Token> Parser::consume_any_of(const std::vector<TokenType> &types) {
  for (const TokenType &type : types) {
    if (check(type)) {
      return advance();
    }
  }
  return std::nullopt;
}

bool Parser::check(const TokenType &type) {
  if (at_end())
    return false;

  return peek().type == type;
}

Token &Parser::advance() {
  if (!at_end())
    current++;
  return previous();
}

bool Parser::at_end() { return peek().type == END_OF_FILE; }

/* Current (unconsumed) token */
Token &Parser::peek() { return tokens[current]; }

Token &Parser::peek_ahead() {
  if (peek().type != END_OF_FILE) {
    return tokens[current + 1];
  }
  return peek();
}

/* Most recently consumed token */
Token &Parser::previous() { return tokens[current - 1]; }

Expr *Parser::comparison() {
  Expr *expr = addition();

  while (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL})) {
    Token &op = previous();
    Expr *right = addition();
    expr = (new Binary(*expr, op, *right));
  }

  return expr;
}

Expr *Parser::addition() {
  Expr *expr = multiplication();

  while (match({MINUS, PLUS})) {
    Token &op = previous();
    Expr *right = multiplication();
    expr = (new Binary(*expr, op, *right));
  }

  return expr;
}

Expr *Parser::multiplication() {
  Expr *expr = modulus();

  while (match({STAR, SLASH})) {
    Token &op = previous();
    Expr *right = modulus();
    expr = (new Binary(*expr, op, *right));
  }

  return expr;
}

Expr* Parser::modulus() {
  Expr *expr = unary();

  while (match({MOD})) {
    Token &op = previous();
    Expr *right = unary();
    expr = (new Binary(*expr, op, *right));
  }

  return expr;
}

Expr *Parser::unary() {
  if (match({BANG, MINUS, AMP, STAR, MINUS_MINUS, PLUS_PLUS})) {
    Token &op = previous();
    Expr *right = unary(); /* Unary is right-recursive */
    return (new Unary(op, *right));
  }

  return call_like();
}

Expr *Parser::call_like() {
  if (match({STATIC_CAST})) {
    consume(LESS, "Expect '<' after static_cast");
    auto typeId = parse_type("static_assert type");
    consume(GREATER, "Expect '>' after static_cast");
    consume(LEFT_PAREN, "Expect '(' after static_cast");
    auto* expr = expression();
    consume(RIGHT_PAREN, "Expect ')' after static_cast");
    return new StaticCast(typeId, expr);
  } else if (match({ALIGNOF})) {
    consume(LEFT_PAREN, "Expect '(' after alignof");
    Expr* expr = new AlignOf(parse_type("alignof requires type-id"));
    consume(RIGHT_PAREN, "Expect ')' after alignof type-id");
    return expr;
  } else if (match({SIZEOF})) {
    // NOTE: We intentionally don't handle unparenthesized sizeof.
    consume(LEFT_PAREN, "Expect '(' after sizeof");
    // Can be a type (or a 'group' expression)
    // Need to loop ahead to find out.
    std::optional<TypeParse> typeId;
    {
      SnoopGuard sg(current, snoopMode);
      try {
        typeId = parse_type("sizeof - try parse type");
      } catch (ParseErr&) {}
    }

    // TODO: - Something like entityTable.resolve(asVar->name).kind ?
    // Or at least make `isType` namespace and template-aware.
    Expr* expr;
    if (typeId && isType(typeId->name)) {
        parse_type(""); // We already know we can parse this
        expr = new SizeOf(*typeId);
    } else {
      expr = new SizeOf(expression());
    }

    consume(RIGHT_PAREN, "Expect ')' after sizeof type-id/expr");
    return expr;
  } else {
    return call();
  }
}

Expr *Parser::call() {
  Expr *expr = primary();

  while (true) {
    if (match({LEFT_PAREN})) {
      /*
       * At this point, everything to left is the callee,
       * so as soon as '(' is seen, try to complete the call
       * expr (or sequence of call exprs). If there is a sequence,
       * all previous calls will become part of the (new) callee.
       *
       * ex - returns_fn()also_returns_fn(arg)returns_val();
       */
      expr = finish_call(expr);
    } else if (match({LEFT_BRACKET})) {
      expr = finish_index_get(expr);
    } else if (match({DOT, ARROW})) {
      Token prev = previous();
      Token name = consume(IDENTIFIER, "Expect property name after '.'.");
      expr = new Get(*expr, name, prev.type);
    } else {
      break;
    }
  }

  return expr;
}

Expr *Parser::finish_index_get(Expr *expr) {
  Expr *index = expression();
  Token bracket = consume(RIGHT_BRACKET, "Expect ']' after index");
  return new GetIndex(*expr, bracket, *index);
}

Expr *Parser::finish_call(Expr *callee) {
  std::vector<Expr *> args;

  if (!check(RIGHT_PAREN)) {
    do {
      if (args.size() >= 8) {
        error(peek(), "Cannot have more than 8 arguments.");
      }
      args.push_back(expression());
    } while (match({COMMA}));
  }

  Token paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");

  return new Call(*callee, paren, args);
}

Expr *Parser::primary() {
  if (match({THIS}))
    return new This(previous());
  if (match({FALSE}))
    return new BoolLiteral(false);
  if (match({TRUE}))
    return new BoolLiteral(true);
  if (match({NIL}))
    return new StrLiteral("nil", true);
  if (match({NUMBER})) {
    return new NumLiteral(previous().literal);
  }
  if (match({STRING}))
    return new StrLiteral(previous().literal);
  if (check(IDENTIFIER) || check(COLON_COLON))
    return new Variable(consume_qualified_identifier("Variable identifier parse"));
  if (match({LEFT_PAREN})) {
    Expr *expr = expression();
    consume(RIGHT_PAREN, "Expect ')' after expression.");
    return (new Grouping(*expr));
  }
  throw error(peek(), "Expect expression.");
}

QualifiedName Parser::consume_qualified_identifier(std::string const& msg) {
  QualifiedName res;

  if (!check(IDENTIFIER) && !check(COLON_COLON)) {
    error(peek(), msg);
  }

  if (check(COLON_COLON)) {
    consume(COLON_COLON, "");
    res.isFullyQualified = true;
  }

  while (check(IDENTIFIER) || check(COLON_COLON)) {
    res.tokens.push_back(consume(IDENTIFIER, msg));
    if (match({COLON_COLON})) {
      // OK, move onto next identifier
    } else {
      break;
    }
  }

  // Is the thing we've parsed so far a template?
  // TODO: This should be based on the (to-be-built)
  // namespace-based name tracker as it doesn't correct
  // account for namespacing. Dirty impl for now:
  if (!isTemplateName(res.token().lexeme)) {
    return res;
  }

  if (match({LESS})) {
    auto lessTok = peek();
    res.tmplInstantiation.emplace();
    if (match({GREATER})) {
      return res;
    }
    do {
      res.tmplInstantiation->push_back(new TypeParse(parse_type("Expect template type")));
    } while (match({COMMA}));
    if (!match({GREATER})) {
      throw error(lessTok, "Unmatched template '<");
    }
  }
  return res;
}

/* Error handling and recovery */

Token &Parser::consume(TokenType type, std::string msg) {
  if (check(type))
    return advance();
  throw error(peek(), msg); /* Report error with current token */
}

ParseErr Parser::error(Token token, std::string msg) {
  if (snoopMode) {
    throw ParseErr(msg);
  }
  Neeilang::error(token, msg);
  return ParseErr(msg);
}
void Parser::synchronize() {
  advance();

  while (!at_end()) {
    switch (peek().type) {
    case CLASS:
    case FN:
    case IF:
    case FOR:
    case WHILE:
    case VAR:
    case PRINT:
    case RETURN:
      return;
    default:
      break;
    }

    advance();
  }
}

bool Parser::isTemplateName(const std::string& name) {
  auto res = templateNames.count(name);
  return res;
}

bool Parser::isType(const QualifiedName& name) const {
  return typeNames.count(name.str());
}

bool Parser::parseStartDecl(bool doCommit) {
  if (at_end()) { return false; }
  TokenType t = peek().type;
  // Something like `int(5)` can start an expression, but in C++
  // if something can be parsed as a decl, it is a decl. Therefore
  // we  try parsing as a decl first if a token canStartDecl.
  if (t == CONST  || t ==  EXTERN || t == CLASS || t == ENUM
    || t == TYPENAME || t == STATIC || t == DECLTYPE || t == AUTO) {
      return true;
  }
  if (t == IDENTIFIER || t == COLON_COLON) {
    auto sr = snoop_qualified_identifier();
    if (sr && doCommit) { commit(sr); }
    return sr && (isType(*sr) || isTemplateName((*sr).token().lexeme)); // TODO: QN lookup
  }
  return false;
}

Parser::SnoopResult<int> Parser::snoopFnDeclarator() {
  SnoopGuard sg(current, snoopMode);
  while (match({STAR, AMP})) {}
  return snoopFnDirectDeclarator();
}

Parser::SnoopResult<int> Parser::snoopFnDirectDeclarator() {
  SnoopGuard sg(current, snoopMode);
  if (match({IDENTIFIER})) {
    // named identifier, doesn't matter if type or not (can be arg name)
  } else if (match({LEFT_PAREN})) {
    if (!snoopFnDeclarator()) {
      return nullSnoopResult<int>();
    }
    consume(RIGHT_PAREN, "direct declarator ')");
  } else {
    return nullSnoopResult<int>();
  }
  // Postfix is () or []
  while (true) {
     if (match({LEFT_PAREN})) {
      if (auto res = snoopFnParameterDeclarationClause(); !res) {
        return res;
      }
      consume(RIGHT_PAREN, "direct declarator ')");
     } else if (match({LEFT_BRACKET})) {
      // TODO: Optionally parse a constexpr here
      consume(RIGHT_BRACKET, "direct declarator ')");
     } else {
      break;
     }
  }
  return snoopResult(1);
}

Parser::SnoopResult<int> Parser::snoopFnParameterDeclaration() {
  SnoopGuard sg(current, snoopMode);
  if (!parseStartDecl(true)) { return nullSnoopResult<int>(); }
  // parseStartDecl true -> we've parsed past the start of the decl
  snoopFnDeclarator(); // optional
  return snoopResult(1);
}

Parser::SnoopResult<int> Parser::snoopFnParameterDeclarationClause() {
    SnoopGuard sg(current, snoopMode);

  if (match({RIGHT_PAREN})) { return snoopResult(1); } // Trivial - func decl wins
  if (!snoopFnParameterDeclaration()) return nullSnoopResult<int>();
  while (match({COMMA})) {
    if (!snoopFnParameterDeclaration()) return nullSnoopResult<int>();
  }
  return snoopResult(1);
}


// Snoop functions
// ---------------
Parser::SnoopResult<QualifiedName> Parser::snoop_qualified_identifier() {
  SnoopGuard sg(current, snoopMode);
  try {
    return snoopResult(consume_qualified_identifier("snoop"));
  } catch (ParseErr&) {}
  return nullSnoopResult<QualifiedName>();
}

Parser::SnoopResult<Specifiers> Parser::snoop_specifiers() {
  SnoopGuard sg(current, snoopMode);
  try {
    snoopResult(consume_specifiers());
  } catch (ParseErr&) {}
  return nullSnoopResult<Specifiers>();
}