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

#ifdef __cplusplus
extern "C" {
#endif
Asm_t *asm_start(Asm_t *a);
void asm_create_instr_only(Asm_t **last, AsmInstr instr);
void asm_create_1arg(Asm_t **last, AsmInstr instr, const char *arg1);
void asm_create(Asm_t **last, AsmInstr instr, const char *arg1,
                const char *arg2);
void asm_print(FILE *f, Asm_t *a);
#ifdef __cplusplus
}
#endif

#endif // __NL_BACKENDS_X86_64_ASM_H__
