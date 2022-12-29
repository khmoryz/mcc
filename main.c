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

  Function *prog = program();

  // Assign offsets to local variables.
  for(Function *fn = prog; fn; fn = fn->next) {
    int offset = 0;
    for (VarList *vl = fn->locals; vl; vl = vl->next) {
      offset += 8;
      vl->var->offset = offset;
    }
    fn->stack_size = offset;
  }

  // for dubug
  // while (node) {
  //   printf("Kind:%d\n", node->kind);
  //   node = node->lhs;
  // }

  // Traverse the AST to emit assembly.
  codegen(prog);

  return 0;
}
