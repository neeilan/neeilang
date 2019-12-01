#ifndef __NL_BACKENDS_X86_64_ASM_H__
#define __NL_BACKENDS_X86_64_ASM_H__

#include <stddef.h>
#include <stdio.h>

typedef const char *AsmInstr;

struct Asm;

typedef struct Asm {
  const char *label;
  const char *instr;
  const char *arg1;
  const char *arg2;
  struct Asm *prev;
  struct Asm *next;
} Asm_t;

Asm_t *asm_start(Asm_t *a);
Asm_t *asm_create_instr(Asm_t *prev, AsmInstr instr);
Asm_t *asm_create_instr_arg1(Asm_t *prev, AsmInstr instr, const char *arg1);
Asm_t *asm_create_instr_arg2(Asm_t *prev, AsmInstr instr, const char *arg1,
                             const char *arg2);
void asm_print(FILE *f, Asm_t *a);

#endif // __NL_BACKENDS_X86_64_ASM_H__
