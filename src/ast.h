//
// ast.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_AST_H
#define IFJ17_AST_H

#include "object.h"
#include "token.h"
#include "vec.h"

/*
 * Nodes.
 */
 #define IFJ17_NODE_LIST \
   n(BLOCK)              \
   n(EXPR_STMT)          \
   n(RETURN)             \
   n(IF)                 \
   n(DIM)                \
   n(DECL)               \
   n(WHILE)              \
   n(CALL)               \
   n(FOR)                \
   n(UNARY_OP)           \
   n(BINARY_OP)          \
   n(BOOL)               \
   n(ID)                 \
   n(ARGS)               \
   n(INT)                \
   n(DOUBLE)             \
   n(STRING)             \
   n(FUNCTION)           \
   n(TYPE)
/*
 * Nodes enum.
 */

typedef enum
{
#define n(node) IFJ17_NODE_##node,
  IFJ17_NODE_LIST
#undef n
} ifj17_node_type;

/*
 * IFJ17 node.
 */

typedef struct
{
  ifj17_node_type type;
  int lineno;
} ifj17_node_t;

/*
 * IFJ17 block node.
 */

typedef struct
{
  ifj17_node_t base;
  ifj17_vec_t *stmts;
} ifj17_block_node_t;

 /*
 * IFJ17 args node.
 */

typedef struct
{
  ifj17_node_t base;
  ifj17_vec_t *vec;
  ifj17_hash_t *hash;
} ifj17_args_node_t;

/*
 * IFJ17 slot access node.
 */

typedef struct
{
  ifj17_node_t base;
  ifj17_node_t *left;
  ifj17_node_t *right;
} ifj17_slot_node_t;

/*
 * IFJ17 unary operation node.
 */

typedef struct
{
  ifj17_node_t base;
  ifj17_token op;
  ifj17_node_t *expr;
  int postfix;
} ifj17_unary_op_node_t;

/*
 * IFJ17 binary operation node.
 */

typedef struct
{
  ifj17_node_t base;
  ifj17_token op;
  ifj17_node_t *left;
  ifj17_node_t *right;
  int let;
} ifj17_binary_op_node_t;

/*
 * IFJ17 int node.
 */

typedef struct
{
  ifj17_node_t base;
  int val;
} ifj17_int_node_t;

/*
 * IFJ17 double node.
 */

typedef struct
{
  ifj17_node_t base;
  double val;
} ifj17_double_node_t;

/*
 * IFJ17 id node.
 */

typedef struct
{
  ifj17_node_t base;
  const char *val;
} ifj17_id_node_t;

/*
 * IFJ17 declaration node.
 */

typedef struct
{
  ifj17_node_t base;
  ifj17_vec_t *vec;
  ifj17_node_t *type;
} ifj17_decl_node_t;

/*
 * IFJ17 dim node.
 */

typedef struct
{
  ifj17_node_t base;
  ifj17_vec_t *vec;
} ifj17_dim_node_t;

/*
 * IFJ17 string node.
 */

typedef struct
{
  ifj17_node_t base;
  const char *val;
} ifj17_string_node_t;

/*
 * IFJ17 call node.
 */

typedef struct
{
  ifj17_node_t base;
  ifj17_node_t *expr;
  ifj17_args_node_t *args;
} ifj17_call_node_t;

/*
 * IFJ17 function node.
 */

typedef struct
{
  ifj17_node_t base;
  const char *name;
  ifj17_node_t *type;
  ifj17_block_node_t *block;
  ifj17_vec_t *params;
} ifj17_function_node_t;

/*
 * IFJ17 type node.
 */

typedef struct
{
  ifj17_node_t base;
  const char *name;
  ifj17_vec_t *fields;
} ifj17_type_node_t;

/*
 * IFJ17 if stmt node.
 */

typedef struct
{
  ifj17_node_t base;
  ifj17_node_t *expr;
  ifj17_block_node_t *block;
  ifj17_block_node_t *else_block;
  ifj17_vec_t *else_ifs;
} ifj17_if_node_t;

/*
 * IFJ17 while loop stmt node.
 */

typedef struct
{
  ifj17_node_t base;
  ifj17_node_t *expr;
  ifj17_block_node_t *block;
} ifj17_while_node_t;

/*
 * IFJ17 return node.
 */

typedef struct
{
  ifj17_node_t base;
  ifj17_node_t *expr;
} ifj17_return_node_t;

// prototypes

ifj17_object_t *ifj17_node(ifj17_node_t *node);

ifj17_block_node_t *ifj17_block_node_new(int lineno);

ifj17_function_node_t *ifj17_function_node_new(const char *name, ifj17_node_t *type,
                                               ifj17_block_node_t *block,
                                               ifj17_vec_t *params, int lineno);

ifj17_function_node_t *ifj17_function_node_new_from_expr(ifj17_node_t *expr,
                                                         ifj17_vec_t *params,
                                                         int lineno);

ifj17_call_node_t *ifj17_call_node_new(ifj17_node_t *expr, int lineno);

ifj17_unary_op_node_t *ifj17_unary_op_node_new(ifj17_token op, ifj17_node_t *expr,
                                               int postfix, int lineno);

ifj17_binary_op_node_t *ifj17_binary_op_node_new(ifj17_token op, ifj17_node_t *left,
                                                 ifj17_node_t *right, int lineno);

ifj17_id_node_t *ifj17_id_node_new(const char *val, int lineno);

ifj17_decl_node_t *ifj17_decl_node_new(ifj17_vec_t *vec, ifj17_node_t *type,
                                       int lineno);

ifj17_int_node_t *ifj17_int_node_new(int val, int lineno);

ifj17_double_node_t *ifj17_double_node_new(double val, int lineno);

ifj17_string_node_t *ifj17_string_node_new(const char *val, int lineno);

ifj17_if_node_t *ifj17_if_node_new(ifj17_node_t *expr,
                                   ifj17_block_node_t *block, int lineno);

ifj17_while_node_t *ifj17_while_node_new(ifj17_node_t *expr,
                                         ifj17_block_node_t *block, int lineno);

ifj17_return_node_t *ifj17_return_node_new(ifj17_node_t *expr, int lineno);

ifj17_args_node_t *ifj17_args_node_new(int lineno);

ifj17_type_node_t *ifj17_type_node_new(const char *name, int lineno);

#endif /* IFJ17_AST_H */
