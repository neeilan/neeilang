#include "backends/x86-64/asm.h"

#include <stdio.h>
#include <stdlib.h>

Asm_t *asm_start(Asm_t *a) {
  Asm_t *curr = a;
  while (curr && curr->prev)
    curr = curr->prev;
  return curr;
}

Asm_t *asm_create_instr_only(Asm_t *prev, AsmInstr instr) {
  Asm_t *a = (Asm_t *)malloc(sizeof(Asm_t));
  a->instr = instr;
  a->prev = prev;

  a->next = NULL;
  a->label = NULL;
  a->arg1 = NULL;
  a->arg2 = NULL;
  return a;
}

Asm_t *asm_create_1arg(Asm_t *prev, AsmInstr instr, const char *arg1) {
  Asm_t *a = asm_create_instr_only(prev, instr);
  a->arg1 = arg1;
  return a;
}

Asm_t *asm_create(Asm_t *prev, AsmInstr instr, const char *arg1,
                  const char *arg2) {
  Asm_t *a = asm_create_1arg(prev, instr, arg1);
  a->arg2 = arg2;
  return a;
}

void asm_print(FILE *f, Asm_t *a) {
  while (a) {
    if (a->label) {
      fprintf(f, "%s:\n", a->label);
    }
    fprintf(f, "%s", a->instr);
    if (a->arg1)
      fprintf(f, " %s", a->arg1);
    if (a->arg2)
      fprintf(f, ", %s", a->arg2);
    fprintf(f, "\n");
    a = a->next;
  }
}
