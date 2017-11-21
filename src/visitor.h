//
// visitor.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_VISITOR_H
#define IFJ17_VISITOR_H

#include "ast.h"

/*
 * Visit the given `node`.
 */

#define visit(node) ifj17_visit(self, node)

/*
 * Visitor struct.
 */

typedef struct ifj17_visitor
{
  void *data;
  void (*visit_block)(struct ifj17_visitor *self, ifj17_block_node_t *node);
  void (*visit_id)(struct ifj17_visitor *self, ifj17_id_node_t *node);
  void (*visit_int)(struct ifj17_visitor *self, ifj17_int_node_t *node);
  void (*visit_double)(struct ifj17_visitor *self, ifj17_double_node_t *node);
  void (*visit_string)(struct ifj17_visitor *self, ifj17_string_node_t *node);
  // void (*visit_slot)(struct ifj17_visitor *self, ifj17_slot_node_t *node);
  void (*visit_call)(struct ifj17_visitor *self, ifj17_call_node_t *node);
  void (*visit_while)(struct ifj17_visitor *self, ifj17_while_node_t *node);
  void (*visit_unary_op)(struct ifj17_visitor *self, ifj17_unary_op_node_t *node);
  void (*visit_binary_op)(struct ifj17_visitor *self, ifj17_binary_op_node_t *node);
  void (*visit_function)(struct ifj17_visitor *self, ifj17_function_node_t *node);
  // void (*visit_array)(struct ifj17_visitor *self, ifj17_array_node_t *node);
  // void (*visit_hash)(struct ifj17_visitor *self, ifj17_hash_node_t *node);
  void (*visit_return)(struct ifj17_visitor *self, ifj17_return_node_t *node);
  void (*visit_decl)(struct ifj17_visitor *self, ifj17_decl_node_t *node);
  void (*visit_if)(struct ifj17_visitor *self, ifj17_if_node_t *node);

  // TODO: discover why the program crashes if i put this somewhere else
  // void (*visit_subscript)(struct ifj17_visitor *self, ifj17_subscript_node_t *node);
  void (*visit_type)(struct ifj17_visitor *self, ifj17_type_node_t *node);
  // void (*visit_let)(struct ifj17_visitor *self, ifj17_let_node_t *node);
  // void (*visit_use)(struct ifj17_visitor *self, ifj17_use_node_t *node);
} ifj17_visitor_t;

// prototypes

void ifj17_visit(ifj17_visitor_t *self, ifj17_node_t *node);

#endif /* IFJ17_VISITOR_H */
