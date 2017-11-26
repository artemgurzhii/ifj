//
// visitor.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "visitor.h"

/*
 * Visit the node when a `type` callback is present.
 */

#define VISIT(type)                                                                 \
  if (!self->visit_##type)                                                          \
    return;                                                                         \
  self->visit_##type(self, (ifj17_##type##_node_t *)node);                          \
  break;

/*
 * Visit `node`, invoking the associated callback.
 */

void ifj17_visit(ifj17_visitor_t *self, ifj17_node_t *node) {
  switch (node->type) {
  case IFJ17_NODE_BLOCK:
    VISIT(block);
  case IFJ17_NODE_ID:
    VISIT(id);
  case IFJ17_NODE_DECL:
    VISIT(decl);
  case IFJ17_NODE_DIM:
    VISIT(dim);
  case IFJ17_NODE_INT:
    VISIT(int);
  case IFJ17_NODE_DOUBLE:
    VISIT(double);
  case IFJ17_NODE_STRING:
    VISIT(string);
  case IFJ17_NODE_SUBSCRIPT:
    VISIT(subscript);
  case IFJ17_NODE_SLOT:
    VISIT(slot);
  case IFJ17_NODE_CALL:
    VISIT(call);
  case IFJ17_NODE_IF:
    VISIT(if);
  case IFJ17_NODE_WHILE:
    VISIT(while);
  case IFJ17_NODE_UNARY_OP:
    VISIT(unary_op);
  case IFJ17_NODE_BINARY_OP:
    VISIT(binary_op);
  case IFJ17_NODE_DECLARE:
    VISIT(declare);
  case IFJ17_NODE_FUNCTION:
    VISIT(function);
  case IFJ17_NODE_SCOPE:
    VISIT(scope);
  case IFJ17_NODE_TYPE:
    VISIT(type);
  case IFJ17_NODE_ARRAY:
    VISIT(array);
  case IFJ17_NODE_HASH:
    VISIT(hash);
  case IFJ17_NODE_RETURN:
    VISIT(return );
  }
}
