#include <string>
#include <vector>

#include "ast.h"

namespace ast {

using std::string;
using std::vector;

class Expression {};
class Statement {};

class Program {
public:
  vector<Statement*> stmts;
};


class ExpressionStatement : public Statement {
public:
  Expression* expression;
};

class AssignmentStatement : public Statement {
public:
  Expression* lhs;
  Expression* rhs;
};

class BinaryOp : public Expression {
public:
  Expression* lhs;
  Expression* rhs;
  char op;
};

class Variable : public Expression {
public:
  Variable(string s) : name(s) {};
  string name;
};

class IntLiteral : public Expression {
public:
  IntLiteral(string s) : value(s) {};
  string value;
};

};

using namespace ast;

int main() {

  Program p;

  auto four = new IntLiteral("4");
  auto five = new IntLiteral("5");

  auto fourPlusFiveExpr = new BinaryOp();
  fourPlusFiveExpr->lhs = four;
  fourPlusFiveExpr->rhs = five;
  fourPlusFiveExpr->op = '+';

  auto xGetsFourPlusFive = new AssignmentStatement();
  xGetsFourPlusFive->rhs = fourPlusFiveExpr;
  xGetsFourPlusFive->lhs = new Variable("x"); 

  p.stmts.push_back(xGetsFourPlusFive); 

  return 0;
}
