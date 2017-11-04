//
// disasm.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_DISASM_H
#define IFJ17_DISASM_H

#include "opcodes.h"
#include "vm.h"
#include <stdio.h>

/*
 * Dump disassembled program to stdout.
 *
 * TODO: return a string
 */

void ifj17_dump(ifj17_vm_t *vm) {
  ifj17_instruction_t *ip = vm->main->ip;
  ifj17_instruction_t i;
  int registers[32] = {0};

  for (;;)
  {
    i = *ip++;
    printf("%10s ", ifj17_op_strings[OP(i)]);
    switch (OP(i))
    {
    // -
    case IFJ17_OP_HALT:
      printf("\n");
      return;

    // op : sBx
    case IFJ17_OP_JMP:
      printf("%d\n", B(i));
      break;

    // op : R(A) RK(B)
    case IFJ17_OP_LOADK:
    case IFJ17_OP_LOADB:
      printf("%d %d; %d\n", A(i), B(i), K(B(i)));
      break;

    // op : R(A) RK(B) RK(C)
    case IFJ17_OP_ADD:
    case IFJ17_OP_SUB:
    case IFJ17_OP_DIV:
    case IFJ17_OP_MUL:
    case IFJ17_OP_MOD:
    case IFJ17_OP_POW:
    case IFJ17_OP_LT:
    case IFJ17_OP_LTE:
      printf("%d %d %d; %d %d\n", A(i), B(i), C(i), RK(B(i)), RK(C(i)));
      break;
    }
  }
}

#endif
