//
// ast.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "ast.h"
#include "hash.h"
#include "internal.h"
#include "vec.h"

/*
 * Alloc a ifj17 value and assign the given `node`.
 */

ifj17_object_t *ifj17_node(ifj17_node_t *node) {
  ifj17_object_t *self = malloc(sizeof(ifj17_object_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->type = IFJ17_TYPE_NODE;
  self->value.as_pointer = node;

  return self;
}

/*
 * Alloc and initialize a new block node.
 */

ifj17_block_node_t *ifj17_block_node_new(int lineno) {
  ifj17_block_node_t *self = malloc(sizeof(ifj17_block_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_BLOCK;
  self->base.lineno = lineno;
  self->stmts = ifj17_vec_new();

  return self;
}

/*
 * Alloc and initialize a new args node.
 */

ifj17_args_node_t *ifj17_args_node_new(int lineno) {
  ifj17_args_node_t *self = malloc(sizeof(ifj17_args_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_ARGS;
  self->base.lineno = lineno;
  self->vec = ifj17_vec_new();
  self->hash = ifj17_hash_new();

  return self;
}

/*
 * Alloc and initialize a new int node with the given `val`.
 */

ifj17_int_node_t *ifj17_int_node_new(int val, int lineno) {
  ifj17_int_node_t *self = malloc(sizeof(ifj17_int_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_INT;
  self->base.lineno = lineno;
  self->val = val;

  return self;
}

/*
 * Alloc and initialize a new double node with the given `val`.
 */

ifj17_double_node_t *ifj17_double_node_new(double val, int lineno) {
  ifj17_double_node_t *self = malloc(sizeof(ifj17_double_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_DOUBLE;
  self->base.lineno = lineno;
  self->val = val;

  return self;
}

/*
 * Alloc and initialize a new id node with the given `val`.
 */

ifj17_id_node_t *ifj17_id_node_new(const char *val, int lineno) {
  ifj17_id_node_t *self = malloc(sizeof(ifj17_id_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_ID;
  self->base.lineno = lineno;
  self->val = val;

  return self;
}

/*
 * Alloc and initialize a new declaration node with the
 * given `name`, `type`, and `val`.
 */

ifj17_decl_node_t *ifj17_decl_node_new(ifj17_vec_t *vec, ifj17_node_t *type,
                                       int lineno) {
  ifj17_decl_node_t *self = malloc(sizeof(ifj17_decl_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_DECL;
  self->base.lineno = lineno;
  self->vec = vec;
  self->type = type;

  return self;
}

/*
 * Alloc and initialize a new dim node with the
 * given `decl` and `val`.
 */

ifj17_dim_node_t *ifj17_dim_node_new(ifj17_vec_t *vec, int lineno) {
  ifj17_dim_node_t *self = malloc(sizeof(ifj17_dim_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_DIM;
  self->base.lineno = lineno;
  self->vec = vec;

  return self;
}

/*
 * Alloc and initialize a new string node with the given `val`.
 */

ifj17_string_node_t *ifj17_string_node_new(const char *val, int lineno) {
  ifj17_string_node_t *self = malloc(sizeof(ifj17_string_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_STRING;
  self->base.lineno = lineno;
  self->val = val;

  return self;
}

/*
 * Alloc and initialize a new call node with the given `expr`.
 */

ifj17_call_node_t *ifj17_call_node_new(ifj17_node_t *expr, int lineno) {
  ifj17_call_node_t *self = malloc(sizeof(ifj17_call_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_CALL;
  self->base.lineno = lineno;
  self->expr = expr;
  self->args = ifj17_args_node_new(lineno);

  if (unlikely(!self->args)) {
    return NULL;
  }

  return self;
}

/*
 * Alloc and initialize subscript node with `left` and `right`.
 */

ifj17_subscript_node_t *ifj17_subscript_node_new(ifj17_node_t *left,
                                                 ifj17_node_t *right, int lineno) {
  ifj17_subscript_node_t *self = malloc(sizeof(ifj17_subscript_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_SUBSCRIPT;
  self->base.lineno = lineno;
  self->left = left;
  self->right = right;

  return self;
}

/*
 * Alloc and initialize slot access node with `left` and `right`.
 */

ifj17_slot_node_t *ifj17_slot_node_new(ifj17_node_t *left, ifj17_node_t *right,
                                       int lineno) {
  ifj17_slot_node_t *self = malloc(sizeof(ifj17_slot_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_SLOT;
  self->base.lineno = lineno;
  self->left = left;
  self->right = right;

  return self;
}

/*
 * Alloc and initialize a unary `op` node with `expr` node.
 */

ifj17_unary_op_node_t *ifj17_unary_op_node_new(ifj17_token op, ifj17_node_t *expr,
                                               int postfix, int lineno) {
  ifj17_unary_op_node_t *self = malloc(sizeof(ifj17_unary_op_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_UNARY_OP;
  self->base.lineno = lineno;
  self->op = op;
  self->expr = expr;
  self->postfix = postfix;

  return self;
}

/*
 * Alloc and initialize a binary `op` node with `left` and `right` nodes.
 */

ifj17_binary_op_node_t *ifj17_binary_op_node_new(ifj17_token op, ifj17_node_t *left,
                                                 ifj17_node_t *right, int lineno) {
  ifj17_binary_op_node_t *self = malloc(sizeof(ifj17_binary_op_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_BINARY_OP;
  self->base.lineno = lineno;
  self->op = op;
  self->left = left;
  self->right = right;
  self->let = 0;

  return self;
}

/*
 * Alloc and initialize a new array node.
 */

ifj17_array_node_t *ifj17_array_node_new(int lineno) {
  ifj17_array_node_t *self = malloc(sizeof(ifj17_array_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_ARRAY;
  self->base.lineno = lineno;
  self->vals = ifj17_vec_new();

  return self;
}

/*
 * Alloc and initialize a new hash node.
 */

ifj17_hash_pair_node_t *ifj17_hash_pair_node_new(int lineno) {
  ifj17_hash_pair_node_t *self = malloc(sizeof(ifj17_hash_pair_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_HASH_PAIR;
  self->base.lineno = lineno;
  self->key = NULL;
  self->val = NULL;

  return self;
}

/*
 * Alloc and initialize a new hash node.
 */

ifj17_hash_node_t *ifj17_hash_node_new(int lineno) {
  ifj17_hash_node_t *self = malloc(sizeof(ifj17_hash_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_HASH;
  self->base.lineno = lineno;
  self->pairs = ifj17_vec_new();

  return self;
}

// TODO: Comments
/*
 * Alloc and initialize a new scope
 */

ifj17_scope_node_t *ifj17_scope_node_new(ifj17_block_node_t *block, int lineno) {
  ifj17_scope_node_t *self = malloc(sizeof(ifj17_scope_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_SCOPE;
  self->base.lineno = lineno;
  self->block = block;

  if (unlikely(!self->block)) {
    return NULL;
  }

  return self;
}

/*
 * Alloc and initialize a new function_declare node with the given `name`,
 * `type` and `params`.
 */

ifj17_declare_node_t *ifj17_declare_node_new(const char *name, ifj17_node_t *type,
                                             ifj17_vec_t *params, int lineno) {
  ifj17_declare_node_t *self = malloc(sizeof(ifj17_declare_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_DECLARE;
  self->base.lineno = lineno;
  self->params = params;
  self->type = type;
  self->name = name;

  return self;
}

// TODO: Add comments
ifj17_print_node_t *ifj17_print_node_new(ifj17_vec_t *params, int lineno) {
  ifj17_print_node_t *self = malloc(sizeof(ifj17_print_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_PRINT;
  self->base.lineno = lineno;
  self->params = params;

  return self;
}

/*
 * Alloc and initialize a new function node with the given `name`,
 * `type`, `block` of statements and `params`.
 */

ifj17_function_node_t *ifj17_function_node_new(const char *name, ifj17_node_t *type,
                                               ifj17_block_node_t *block,
                                               ifj17_vec_t *params, int lineno) {
  ifj17_function_node_t *self = malloc(sizeof(ifj17_function_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_FUNCTION;
  self->base.lineno = lineno;
  self->params = params;
  self->block = block;
  self->type = type;
  self->name = name;

  return self;
}

/*
 * Alloc and initialize a new function node with the given `expr`,
 * with an implicit return.
 */

ifj17_function_node_t *ifj17_function_node_new_from_expr(ifj17_node_t *expr,
                                                         ifj17_vec_t *params,
                                                         int lineno) {
  ifj17_function_node_t *self = malloc(sizeof(ifj17_function_node_t));
  if (unlikely(!self)) {
    return NULL;
  }
  self->base.type = IFJ17_NODE_FUNCTION;
  self->base.lineno = lineno;
  self->params = params;

  // block
  self->block = ifj17_block_node_new(lineno);
  if (unlikely(!self->block)) {
    return NULL;
  }

  // return
  ifj17_return_node_t *ret = ifj17_return_node_new(expr, lineno);
  ifj17_vec_push(self->block->stmts, ifj17_node((ifj17_node_t *)ret));

  return self;
}

/*
 * Alloc and initialize a new type noe with the given `name`.
 */

ifj17_type_node_t *ifj17_type_node_new(const char *name, int lineno) {
  ifj17_type_node_t *self = malloc(sizeof(ifj17_type_node_t));
  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_TYPE;
  self->base.lineno = lineno;
  self->name = name;
  self->fields = ifj17_vec_new();

  return self;
}

/*
 * Alloc and initialize a new if stmt node,
 * with required `expr` and `block`.
 */

ifj17_if_node_t *ifj17_if_node_new(ifj17_node_t *expr, ifj17_block_node_t *block,
                                   int lineno) {
  ifj17_if_node_t *self = malloc(sizeof(ifj17_if_node_t));

  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_IF;
  self->base.lineno = lineno;
  self->expr = expr;
  self->block = block;
  self->else_block = NULL;
  self->else_ifs = ifj17_vec_new();

  return self;
}

/*
 * Alloc and initialize a new while loop node,
 * otherwise "while", with required `expr` and `block`.
 */

ifj17_while_node_t *ifj17_while_node_new(ifj17_node_t *expr,
                                         ifj17_block_node_t *block, int lineno) {
  ifj17_while_node_t *self = malloc(sizeof(ifj17_while_node_t));

  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_WHILE;
  self->base.lineno = lineno;
  self->expr = expr;
  self->block = block;

  return self;
}

/*
 * Alloc and initialize a new return node with the given `expr`.
 */

ifj17_return_node_t *ifj17_return_node_new(ifj17_node_t *expr, int lineno) {
  ifj17_return_node_t *self = malloc(sizeof(ifj17_return_node_t));

  if (unlikely(!self)) {
    return NULL;
  }

  self->base.type = IFJ17_NODE_RETURN;
  self->base.lineno = lineno;
  self->expr = expr;

  return self;
}
