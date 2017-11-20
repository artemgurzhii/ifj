//
// vm.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "vm.h"
#include "disasm.h"
#include "internal.h"
#include "object.h"
#include "opcodes.h"

ifj17_object_t *ifj17_eval(ifj17_vm_t *vm) {
  ifj17_dump(vm);
  printf("\n");
  ifj17_instruction_t *ip = vm->main->ip;
  ifj17_instruction_t i;
  int registers[32] = {0};

  for (;;)
  {
    switch (OP(i = *ip++))
    {
    // LOADK
    case IFJ17_OP_LOADK:
      printf("loadk %d\n", K(B(i)));
      R((A(i))) = K(B(i));
      break;

    // LOADB
    case IFJ17_OP_LOADB:
      printf("loadb %d %d %d\n", A(i), K(B(i)), C(i));
      R(A(i)) = K(B(i));
      if (C(i))
        ip++;
      break;

    // ADD
    case IFJ17_OP_ADD:
      R(A(i)) = RK(B(i)) + RK(C(i));
      break;

    // SUB
    case IFJ17_OP_SUB:
      R(A(i)) = RK(B(i)) - RK(C(i));
      break;

    // DIV
    case IFJ17_OP_DIV:
      R(A(i)) = RK(B(i)) / RK(C(i));
      break;

    // MUL
    case IFJ17_OP_MUL:
      R(A(i)) = RK(B(i)) * RK(C(i));
      break;

    // MOD
    case IFJ17_OP_MOD:
      R(A(i)) = RK(B(i)) % RK(C(i));
      break;

    // NEGATE
    case IFJ17_OP_NEGATE:
      R(A(i)) = -R(B(i));
      break;

    // LT
    case IFJ17_OP_LT:
      printf("lt %d %d\n", RK(B(i)), RK(C(i)));
      if (RK(B(i)) < RK(C(i)))
        ip++;
      break;

    // LTE
    case IFJ17_OP_LTE:
      printf("lte %d %d\n", RK(B(i)), RK(C(i)));
      if (RK(B(i)) <= RK(C(i)))
        ip++;
      break;

    // JMP
    case IFJ17_OP_JMP:
      printf("jmp %d\n", B(i));
      ip += B(i);
      break;

    // HALT
    case IFJ17_OP_HALT:
      goto end;
    }
  }

end:
  return ifj17_int_new(R(0));
}

void ifj17_vm_free(ifj17_vm_t *vm) {
  free(vm->main->constants);
  free(vm->main->code);
  free(vm->main);
  free(vm);
}
