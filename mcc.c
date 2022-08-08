#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Tokenizer
//

typedef enum {
  TK_RESERVED,  // Keywords or punctuators,
  TK_NUM,       // Integer literals
  TK_EOF,       // End-of-file markers
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;  // Token kind
  Token *next;     // Next Token
  int val;         // If kind is TK_NUM, its value
  char *str;       // Token string
};

Token *token;

char *user_input;

// Reports an error and exit.
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error with location and exit.
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");  // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Create a new token and add it as the next token of `cur`.
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *token = calloc(1, sizeof(Token));
  token->kind = kind;
  token->str = str;
  cur->next = token;
  return token;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // Skip whitespace characters.
    if (isspace(*p)) {
      p++;
      continue;
    }
    // Punctuator
    if (strchr("+-*/", *p)) {
      cur = new_token(TK_RESERVED, cur, p);
      p++;
      continue;
    }

    // Integer literal
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error_at(p, "invalid token");
  }
  cur = new_token(TK_EOF, cur, p);
  return head.next;
}

//
// Parser
//

typedef enum {
  ND_ADD,  // +
  ND_SUB,  // -
  ND_MUL,  // *
  ND_DIV,  // /
  ND_NUM,  // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
  NodeKind kind;  // Node kind
  Node *lhs;      // Left-hand side
  Node *rhs;      // Right-hand side
  int val;        // Used if kind == ND_NUM
};

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

// Consumes the current token if it matches `op`.
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    return false;
  }
  token = token->next;
  return true;
}

// Ensure that the current token is `op`.
void expect(char op) {
  if (token->kind != TK_RESERVED) {
    error_at(token->str, "expect TK_RESERVED");
  }
  if (token->str[0] != op) {
    error_at(token->str, "expected:%s, but got %s.", op, token->str[0]);
  }
  token = token->next;
}

// Ensure that the current token is TK_NUM.
int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "expected a number");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

Node *expr();
Node *mul();
Node *primary();

// expr = mul ("+" mul | "-" mul)*
Node *expr() {
  Node *node = mul();
  for (;;) {
    if (consume('+')) {
      node = new_binary(ND_ADD, node, mul());
      continue;
    } else if (consume('-')) {
      node = new_binary(ND_SUB, node, mul());
      continue;
    } else {
      return node;
    }
  }
}

// expr = primary ("*" primary | "/" primary)*
Node *mul() {
  Node *node = primary();
  for (;;) {
    if (consume('*')) {
      node = new_binary(ND_MUL, node, primary());
    } else if (consume('/')) {
      node = new_binary(ND_DIV, node, primary());
    } else {
      return node;
    }
  }
}

// primary = num
Node *primary() { return new_num(expect_number()); }

// Code Generator
void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }
  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    default:
      error("invalid node kind. kind: %s", node->kind);
      break;
  }
  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(user_input);

  // for dubug
  // while (token)
  // {
  //   printf("Kind:%d\n", token->kind);
  //   printf("Str:%s\n", token->str);
  //   token = token->next;
  // }

  Node *node = expr();

  // for dubug
  // while (node) {
  //   printf("Kind:%d\n", node->kind);
  //   node = node->lhs;
  // }

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  // Traverse the AST to emit assembly.
  gen(node);

  // A result must be at the top of the stack, so pop it
  // to RAX to make it a program exit code.
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
