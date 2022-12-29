#include "mcc.h"

char *user_input;
Token *token;

// Reports an error and exit.
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error location and exit.
void verror_at(char *loc, char *fmt, va_list ap) {
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error with location and exit.
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
}

// Reports an error location and exit.
void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  if (tok)
    verror_at(tok->str, fmt, ap);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

char *strndup(char *p, int len) {
  char *buf = malloc(len + 1);
  strncpy(buf, p, len);
  buf[len] = '\0';
  return buf;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT) {
    return NULL;
  }
  Token *t = token;
  token = token->next;
  return t;
}

char *expect_ident() {
  if (token->kind != TK_IDENT)
    error_at(token->str, "expected an identifier");
  char *s = strndup(token->str, token->len);
  token = token->next;
  return s;
}

// Create a new token and add it as the next token of `cur`.
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *token = calloc(1, sizeof(Token));
  token->kind = kind;
  token->str = str;
  token->len = len;
  cur->next = token;
  return token;
}

bool startswith(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool is_alnum(char c) { return is_alpha(c) || ('0' <= c && c <= '9'); }

char *starts_with_reserved(char *p) {
  // Keyword
  static char *kw[] = {"return", "if", "else", "while", "for"};

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
    int len = strlen(kw[i]);
    if (startswith(p, kw[i]) && !is_alnum(p[len]))
      return kw[i];
  }

  // Multi-letter punctuator
  static char *ops[] = {"==", "!=", "<=", ">="};

  for (int i=0; i < sizeof(ops) / sizeof(*ops); i++)
    if (startswith(p, ops[i]))
      return ops[i];
  return NULL;
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

    // Keyword or multi-letter punctuator
    char *kw = starts_with_reserved(p);
    if (kw) {
      int len = strlen(kw);
      cur = new_token(TK_RESERVED, cur, p, len);
      p += len;
      continue;
    }

    // Single-Letter punctuator
    if (strchr("+-*/()<>;={},", *p)) {
      cur = new_token(TK_RESERVED, cur, p, 1);
      p++;
      continue;
    }

    // Integer literal
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = q - p;
      continue;
    }

    // Identifier
    if (is_alpha(*p)) {
      char *q = p++;
      while (is_alnum(*p))
        p++;
      cur = new_token(TK_IDENT, cur, q, p - q);
      continue;
    }

    error_at(p, "invalid token");
  }
  cur = new_token(TK_EOF, cur, p, 0);
  return head.next;
}
