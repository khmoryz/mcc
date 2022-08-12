#include "mcc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(user_input);

  // for dubug
  // while (token) {
  //   printf("-----\n");
  //   printf("Kind:%d\n", token->kind);
  //   printf("Str:%s\n", token->str);
  //   printf("Len:%d\n", token->len);
  //   token = token->next;
  // }

  Node *node = program();

  // for dubug
  // while (node) {
  //   printf("Kind:%d\n", node->kind);
  //   node = node->lhs;
  // }

  // Traverse the AST to emit assembly.
  codegen(node);

  return 0;
}
