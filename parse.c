#include "mcc.h"

Var *locals;

Var *find_var(Token *tok) {
  for (Var *var = locals; var; var = var->next)
    if (strlen(var->name) == tok->len && !memcmp(tok->str, var->name, tok->len))
      return var;
  return NULL;
}

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

Node *new_unary(NodeKind kind, Node *lhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  return node;
}

Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *new_var(Var *var) {
  Node *node = new_node(ND_VAR);
  node->var = var;
  return node;
}

Var *push_var(char *name) {
  Var *var = calloc(1, sizeof(Var));
  var->next = locals;
  var->name = name;
  locals = var;
  return var;
}

// Consumes the current token if it matches `op`.
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, strlen(op))) {
    return false;
  }
  token = token->next;
  return true;
}

// Ensure that the current token is `op`.
void expect(char *op) {
  if (token->kind != TK_RESERVED) {
    error_at(token->str, "expect TK_RESERVED");
  }
  if (strlen(op) != token->len) {
    error_at(token->str, "expected length:%d, but got %d.", strlen(op),
             token->len);
  }
  if (memcmp(token->str, op, strlen(op))) {
    error_at(token->str, "expected:%s, but got %s.", op, token->str);
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

Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// program = stmt*
Program *program() {
  locals = NULL;
  Node head;
  head.next = NULL;
  Node *cur = &head;
  while (!at_eof()) {
    cur->next = stmt();
    cur = cur->next;
  }

  Program *prog = calloc(1, sizeof(Program));
  prog->node = head.next;
  prog->locals= locals;
  return prog;
}

// stmt = expr ";" | "return" expr ";"
Node *stmt() {
  if (consume("return")) {
    Node *node = new_unary(ND_RETURN, expr());
    expect(";");
    return node;
  };

  Node *node = new_unary(ND_EXPR_STMT, expr());
  expect(";");
  return node;
}

// expr = assign
Node *expr() { return assign(); }

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = new_binary(ND_ASSIGN, node, assign());
  }
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();
  for (;;) {
    if (consume("==")) {
      node = new_binary(ND_EQ, node, relational());
      continue;
    } else if (consume("!=")) {
      node = new_binary(ND_NE, node, relational());
      continue;
    } else {
      return node;
    }
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();
  for (;;) {
    if (consume("<")) {
      node = new_binary(ND_LT, node, add());
    } else if (consume("<=")) {
      node = new_binary(ND_LE, node, add());
    } else if (consume(">")) {
      node = new_binary(ND_LT, add(), node);
    } else if (consume(">=")) {
      node = new_binary(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();
  for (;;) {
    if (consume("+")) {
      node = new_binary(ND_ADD, node, mul());
      continue;
    } else if (consume("-")) {
      node = new_binary(ND_SUB, node, mul());
      continue;
    } else {
      return node;
    }
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();
  for (;;) {
    if (consume("*")) {
      node = new_binary(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_binary(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

// unary = ("+" | "-")? primary
Node *unary() {
  if (consume("+")) {
    return primary();
  } else if (consume("-")) {
    return new_binary(ND_SUB, new_num(0), primary());
  }
  return primary();
}

// primary = "(" expr ")" | num | ident
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if(tok) {
    Var *var = find_var(tok);
    if(!var)
      var = push_var(strndup(tok->str, tok->len));
    return new_var(var);
  }

  return new_num(expect_number());
}
