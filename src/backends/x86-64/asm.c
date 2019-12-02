#include "backends/x86-64/asm.h"

#include <stdio.h>
#include <stdlib.h>

Asm_t *asm_start(Asm_t *a) {
  Asm_t *curr = a;
  while (curr && curr->prev)
    curr = curr->prev;
  return curr;
}

void asm_create_instr_only(Asm_t **last, AsmInstr instr) {
  Asm_t *a = (Asm_t *)malloc(sizeof(Asm_t));
  a->instr = instr;
  a->prev = *last;
  if (*last)
    (*last)->next = a;
  *last = a;

  a->next = NULL;
  a->label = NULL;
  a->arg1 = NULL;
  a->arg2 = NULL;
}

void asm_create_1arg(Asm_t **last, AsmInstr instr, const char *arg1) {
  asm_create_instr_only(last, instr);
  (*last)->arg1 = arg1;
}

void asm_create(Asm_t **last, AsmInstr instr, const char *arg1,
                const char *arg2) {
  asm_create_1arg(last, instr, arg1);
  (*last)->arg2 = arg2;
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
