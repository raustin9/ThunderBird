/*
 * ast.hh
 *
 * This file contains the structure for the abstract syntax tree of ThunderBird
 */

#pragma once
#ifndef AST_
#define AST_

#include "token.hh"
#include "symboltable.hh"
#include <vector>
#include <string>
#include <memory>

// Node in the AST
class Node {
  public:
    virtual ~Node() = default;
    virtual void print() {};
};

// Statement Node
// Does not contain a value, and excutes a command
class Statement : public Node {
  public:
    virtual ~Statement() = default;
    void print() override;
};

// Expression node
// Evaluates to a value that will be held
class Expression : public Node {
  public:
    DataType data_type;
    virtual ~Expression() = default;
    void print() override;
};

// Statement node for an expression
// Wrapper so that something like "x + 15;" is valid code on its own
class ExpressionStatement : public Statement {
  public:
    token_t token;                    // first token of the expression
    std::unique_ptr<Expression> expr; // holds the expression

    ExpressionStatement(
      token_t token,
      std::unique_ptr<Expression> expr
    ) : token(token) , expr(std::move(expr)) {}
    void print() override;
};

//// Expression with an infix operator
class BinaryExpr : public Expression {
  public:
    token_t op;
    std::unique_ptr<Expression> LHS;
    std::unique_ptr<Expression> RHS;
    BinaryExpr(
        token_t op,
        std::unique_ptr<Expression> LHS,
        std::unique_ptr<Expression> RHS
      ) : op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    void print() override;
};

// Prefix or unary operator
class PrefixOperator {
  public:
    token_t token;
    std::string op;
    std::unique_ptr<Expression> RHS;
};

// Node for assigning a variable
// "x = 3 + 20"
class VariableAssignment : public Expression {
  public:
  token_t op; // "="
  std::unique_ptr<Expression> variable;
  std::unique_ptr<Expression> RHS;
  VariableAssignment(
      token_t op,
      std::unique_ptr<Expression> variable,
      std::unique_ptr<Expression> RHS
    ) : op(op), variable(std::move(variable)), RHS(std::move(RHS)) {}
  void print() override;
};

// Integer Expression Node
// Node that is a integer literal like "1" or "300"
class IntegerExpr : public Expression {
  public:
    long long value;
    DataType data_type = TYPE_INT;
    IntegerExpr(long long value) : value(value) {};
    void print() override;
};

// Float Expression Node
// Node that is a floating point literal like "1.0" or "3.14"
class FloatExpr : public Expression {
  public:
    double value;
    DataType data_type = TYPE_FLOAT;
    FloatExpr(double value) : value(value) {};
    void print() override;
};

// Statement node for code block
// Code block is contained within '{}'
// {
//   let int x = 1;
// }
class CodeBlock : public Statement {
  public:
    std::vector <std::unique_ptr<Statement> > body; // the body of code of this scope
    std::shared_ptr<SymbolTable> symbol_table;      // the symbol table of identifiers for this code block's scope
    std::shared_ptr<Statement> parent_scope;        // the parent scope of this code block. Can be function or global scope
    // add a field for an inner scope

    CodeBlock() {
      this->symbol_table = std::make_shared<SymbolTable>();
    }
    void print() override;
};

// Boolean Expression Node
// Node that is a boolean value like 'true' or 'false'
class BooleanExpr : public Expression {
  public:
    bool value;
    DataType data_type = TYPE_BOOL;

    BooleanExpr(
      bool value
    ) : value(value) {}
    void print() override;
};

// Function Call Expression node
// This node is for when a function is called in an expression
// "func1(x, y);"
// "let int x = add(1, 2) + 4;"
class FunctionCallExpr : public Expression {
  public:
    std::string name;
    std::vector <std::unique_ptr<Expression> > args;

    FunctionCallExpr(
      std::string &name,
      std::vector <std::unique_ptr<Expression> > args
    ) : name(name), args(std::move(args)) {}
    void print() override;
};

// Identifier class that holds the name and data type of the identifier
// identifier can be either a function or a variable
class IdentifierExpr : public Expression {
  public:
    std::string name;   // name of the identifie
    DataType data_type; // data type of the identifier (return type for function, stored type for variable)

    void print() override;
};

// Variable Expression node
// Is an expression because a variable does contain a value that it is assigned to
class VariableExpr : public Expression {
  public:
    std::string name;    // name of the variable
    DataType data_type;  // data type of the variable -- float or int

    VariableExpr(const std::string &name, DataType data_type) : name(name), data_type(data_type) {}
    void print() override;
};

// Statement node for let statements for variable declaration
// "let x = 3;"
class LetStmt : public Statement {
  public:
    token_t token;                          // "let" token
    std::unique_ptr<Expression> variable;   // expression of the variable being declared
    unsigned decl_line;                     // the line of the statement
    std::unique_ptr<Expression> var_assign; // expression that variable will be assigned to
   
    LetStmt(
        token_t token,
        std::unique_ptr<Expression> variable,
        std::unique_ptr<Expression> var_assign
      ) : token(token), variable(std::move(variable)), var_assign(std::move(var_assign)) {}
    void print() override;
    std::unique_ptr<SymbolTableEntry> get_st_entry();
};

// Statement node for return statements
// "return x + 5;"
class ReturnStmt : public Statement {
  public:
    token_t token;
    std::unique_ptr<Expression> ret_val; // return value of the function
    
    ReturnStmt(
      token_t token,
      std::unique_ptr<Expression> ret_val
    ) : token(token), ret_val(std::move(ret_val)) {}
    void print() override;
};

// Statement node for if statements
// "if (x == 1) {
//    print("true");
//  } else if (x == 2) {
//    print("x = 2");
//  } else {
//    print("false");
//  }
//  Notes:
//    The field "condition" is evaluated, and if it returns 'true' then the "consequence" field is evaluated
//    If the condition returns false, then we then evaluate the condition of the "alternative" field
//    The reason that the "alternative" field is another conditional is because it allows us to chain if clauses together
//    This is how we implement "else if" statements
class Conditional : public Statement {
  public:
    token_t token;
    std::shared_ptr<Statement> consequence;             // body of if statement
    std::unique_ptr<Expression> condition;              // the condition to evaluate
    std::shared_ptr<Statement> alternative;             // the conditional to evaluate if the condition is not true -- this is how we do else-if
    std::shared_ptr<Statement> parent;                  // parent scope of the conditional

    Conditional(
      token_t token,
      std::shared_ptr<Statement> consequence,
      std::unique_ptr<Expression> condition,
      std::shared_ptr<Statement> alternative
    ) : token(token), 
        consequence(std::move(consequence)),
        condition(std::move(condition)),
        alternative(std::move(alternative))
      {}
    void print() override;
};

// Statement node for while loop
// This will loop while the condition remains true and break when false
// "while (x  < 4)..."
class WhileLoop : public Statement {
  public:
    token_t token;
    std::unique_ptr<Expression> condition;
    std::shared_ptr<Statement> loop_body;
    std::shared_ptr<Statement> parent;    // symbol table of the loop 

    WhileLoop(
      token_t token,
      std::unique_ptr<Expression> condition,
      std::shared_ptr<Statement> loop_body
    ) : token(token), 
        condition(std::move(condition)),
        loop_body(std::move(loop_body))
      {}
    void print() override;
};

// Statement node for a for loop
// This will perform the intialization before entering the loop
// Then check the condition, and if true it will run the loop body
// At end of loop body, it will perform the action
// "for (let int x = 0; x < 13; x = x + 1) {...}"
class ForLoop : public Statement {
  public:
    token_t token;                             // the token that represents the command
    std::unique_ptr<Statement> initialization; // the initialization statement in the for loop
    std::unique_ptr<Expression> condition;     // the condition that the loop runs until fulfilled
    std::unique_ptr<Expression> action;        // the action that gets taken at the end of each iteration
    std::shared_ptr<Statement> loop_body;      // the body of the for loop
    std::shared_ptr<Statement> parent;         // symbol table of the loop 

    ForLoop(
      token_t token,
      std::unique_ptr<Statement> initialization,
      std::unique_ptr<Expression> condition,
      std::unique_ptr<Expression> action,
      std::shared_ptr<Statement> loop_body
    ) : token(token) , initialization(std::move(initialization)), condition(std::move(condition)) , action(std::move(action)), loop_body(std::move(loop_body)) {}
    void print() override;
};

// Class for function prototypes
class Prototype {
  public:
  std::string name;
  DataType ret_type;
  std::vector <IdentifierExpr> params;

  Prototype(
    std::string name,
    DataType ret_type,
    std::vector <IdentifierExpr> params
  ) : name(std::move(name)), ret_type(ret_type), params(std::move(params)) {}
};

// Class for function declarations
class FunctionDecl : public Statement {
  public:
    bool is_entry;                                 // true if it is the entry point to the program false otherwise
    std::shared_ptr<Statement> func_body;          // a CodeBlock that contains the body of the function
    std::unique_ptr<Prototype> prototype;          // the prototype of the function
    std::shared_ptr<class Program> parent;         // parent scope of the function -- global scope
    

    FunctionDecl(
      bool is_entry,
      std::shared_ptr<Statement> func_body,
      std::unique_ptr<Prototype> prototype
    ) : is_entry(is_entry), func_body(std::move(func_body)), prototype(std::move(prototype)) {}
    void print() override;
};

//class Function : public Node {
//  public:
//    std::unique_ptr<Prototype> prototype;           // function prototype
//    std::vector <std::unique_ptr<Statement> > body; // list of statements that make up the function body
//
//    Function(
//      std::unique_ptr<Prototype> prototype,
//      std::vector <std::unique_ptr<Statement> > body
//    ) : prototype(std::move(prototype)), body(std::move(body)) {}
//};

// Program Node in the AST
// should be the root node of the tree
class Program : public Node {
  public:
    std::unique_ptr<Statement> entry_point; // Potentially use to define entry point of program
    std::vector<std::shared_ptr<Statement> > statements; // top level of the program is a list of statements
    std::shared_ptr<SymbolTable> symbol_table;           // the symbol table for the global scope
    void print() override;

    Program() {
      this->symbol_table = std::make_shared<SymbolTable>();
    }
};

#endif
