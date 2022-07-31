#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum
{
  TK_RESERVED, // Keywords or punctuators,
  TK_NUM,      // Integer literals
  TK_EOF,      // End-of-file markers
} TokenKind;

typedef struct Token Token;
struct Token
{
  TokenKind kind; // Token kind
  Token *next;    // Next Token
  int val;        // If kind is TK_NUM, its value
  char *str;      // Token string
};

Token *token;

char *user_input;

// Reports an error and exit.
void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error with location and exit.
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Create a new token and add it as the next token of `cur`.
Token *new_token(TokenKind kind, Token *cur, char *str)
{
  Token *token = calloc(1, sizeof(Token));
  token->kind = kind;
  token->str = str;
  cur->next = token;
  return token;
}

Token *tokenize(char *p)
{
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p)
  {
    // Skip whitespace characters.
    if (isspace(*p))
    {
      p++;
      continue;
    }
    // Punctuator
    if (*p == '+' || *p == '-')
    {
      cur = new_token(TK_RESERVED, cur, p);
      p++;
      continue;
    }

    // Integer literal
    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error_at(cur->str, "invalid token");
  }
  cur = new_token(TK_EOF, cur, p);
  return head.next;
}

// Consumes the current token if it matches `op`.
bool consume(char op)
{
  if (token->kind != TK_RESERVED)
  {
    error_at(token->str, "op is not TK_RESERVED");
    return false;
  }
  if (token->str[0] != op)
  {
    return false;
  }
  token = token->next;
  return true;
}

// Ensure that the current token is `op`.
void expect(char op)
{
  if (token->kind != TK_RESERVED)
  {
    error_at(token->str, "expect TK_RESERVED");
  }
  if (token->str[0] != op)
  {
    error_at(token->str, "expected:%s, but got %s.", op, token->str[0]);
  }
  token = token->next;
}

// Ensure that the current token is TK_NUM.
int expect_number()
{
  if (token->kind != TK_NUM)
  {
    error_at(token->str, "expected a number");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof()
{
  return token->kind == TK_EOF;
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
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

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  printf("  mov rax, %d\n", expect_number());

  while (!at_eof())
  {
    if (consume('+'))
    {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }
  printf("  ret\n");
  return 0;
}
