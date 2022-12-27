#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// tokenize.c
//

// Token
typedef enum {
  TK_RESERVED,  // Keywords or punctuators
  TK_IDENT,     // Identifier
  TK_NUM,       // Integer literals
  TK_EOF,       // End-of-file markers
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;  // Token kind
  Token *next;     // Next Token
  int val;         // If kind is TK_NUM, its value
  char *str;       // Token string
  int len;         // Token length
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

char *strndup(char *p, int len);

Token *consume_ident();
Token *tokenize();

extern Token *token;
extern char *user_input;

//
// parse.c
//

// Local variable
typedef struct Var Var;

struct Var {
  Var *next;
  char *name; // Variable name
  int offset; // Offset from RBP
};


// AST node
typedef enum {
  ND_ADD,        // +
  ND_SUB,        // -
  ND_MUL,        // *
  ND_DIV,        // /
  ND_EQ,         // ==
  ND_NE,         // !=
  ND_LT,         // <
  ND_LE,         // <=
  ND_ASSIGN,     // =
  ND_RETURN,     // "return"
  ND_IF,         // "if"
  ND_EXPR_STMT,  // Expression statement
  ND_VAR,        // Variable
  ND_NUM,        // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
  NodeKind kind;  // Node kind
  Node *next;     // Next node

  Node *lhs;      // Left-hand side
  Node *rhs;      // Right-hand side

  // "if" statement
  Node *cond;
  Node *then;
  Node *els;

  Var *var;       // Used if kind == ND_VAR (not necessary??)
  int val;        // Used if kind == ND_NUM
};

typedef struct {
  Node *node;
  Var *locals;
  int stack_size;
} Program;

Program *program();

//
// codegen.c
//

void codegen(Program *node);
