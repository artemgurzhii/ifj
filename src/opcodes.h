//
// opcodes.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_OPCODES_H_H
#define IFJ17_OPCODES_H_H

/*
 * Opcodes.
 */

 #define IFJ17_OP_LIST \
   o(HALT, "halt")     \
   o(JMP, "jmp")       \
   o(LOADK, "loadk")   \
   o(LOADB, "loadb")   \
   o(MOVE, "move")     \
   o(EQ, "eq")         \
   o(LT, "lt")         \
   o(LTE, "lte")       \
   o(ADD, "add")       \
   o(SUB, "sub")       \
   o(DIV, "div")       \
   o(MUL, "mul")       \
   o(MOD, "mod")       \
   o(POW, "pow")       \
   o(NEGATE, "negate") \
   o(BIT_SHL, "bshl")  \
   o(BIT_SHR, "bshr")  \
   o(BIT_AND, "band")  \
   o(BIT_OR, "bor")    \
   o(BIT_XOR, "bxor")
/*
 * Opcodes enum.
 */

typedef enum
{
#define o(op, str) IFJ17_OP_##op,
  IFJ17_OP_LIST
#undef o
} ifj17_op_t;

/*
 * Opcode strings.
 */

static char *ifj17_op_strings[] = {
#define o(op, str) str,
    IFJ17_OP_LIST
#undef o
};

#endif /* IFJ17_OPCODES_H_H */
