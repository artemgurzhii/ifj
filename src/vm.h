//
// vm.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_VM_H
#define IFJ17_VM_H

#include "ast.h"
#include <stdint.h>

/*
 * Instruction.
 */

typedef uint32_t ifj17_instruction_t;

/*
 * IFJ17 activation record.
 */

typedef struct
{
  ifj17_instruction_t *ip;
  ifj17_instruction_t *code; // TODO: pointer to single malloc()?
  int nconstants;
  int *constants;
} ifj17_activation_t;

/*
 * IFJ17 VM.
 */

typedef struct
{
  ifj17_activation_t *main;
  ifj17_instruction_t *jump;
} ifj17_vm_t;

/*
 *   8    8   8   8
 * +----------------+
 * | op | a | b | c |
 * +----------------+
 */

#define ABC(op, a, b, c) (IFJ17_OP_##op << 24 | (a) << 16 | (b) << 8 | (c))

/*
 *   8    8    16
 * +----------------+
 * | op | a |   b   |
 * +----------------+
 */

#define AB(op, a, b) (IFJ17_OP_##op << 24 | (a) << 16 | (b) << 8)

/*
 * Opcode.
 */

#define OP(i) ((i) >> 24 & 0xff)

/*
 * Operand A.
 */

#define A(i) ((i) >> 16 & 0xff)

/*
 * Operand B.
 */

#define B(i) ((i) >> 8 & 0xff)

/*
 * Operand C.
 */

#define C(i) ((i)&0xff)

/*
 * Register n.
 */

#define R(n) registers[n]

/*
 * Constant n.
 */

#define K(n) vm->main->constants[(n)-32]

/*
 * Register or constant.
 */

// TODO: MSB
#define RK(n) ((n) < 32 ? R(n) : K(n))

// protoypes

ifj17_object_t *ifj17_eval(ifj17_vm_t *vm);

void ifj17_vm_free(ifj17_vm_t *vm);

#endif /* IFJ17_VM_H */
