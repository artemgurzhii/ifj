//
// prettyprint.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "ast.h"
#include "prettyprint.h"
#include "vec.h"
#include "visitor.h"
#include <stdio.h>

// indentation level

static int indents = 0;

// print function

int (*print_func)(const char *format, ...);

void ifj17_set_prettyprint_func(int (*func)(const char *format, ...)) {
  print_func = func;
}

// output indentation

#define INDENT                                                                      \
  for (int j = 0; j < indents; ++j)                                                 \
  print_func("  ")

/*
 * Escape chars.
 */

static char escapes[] = {'a', 'b', 't', 'n', 'v', 'f', 'r'};

/*
 * Return the length of an "inspected" string
 * including the null byte.
 */

static int inspect_length(const char *str) {
  int len = 0;
  for (int i = 0; str[i]; ++i) {
    switch (str[i]) {
    case '\a':
    case '\b':
    case '\e':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
      len += 2;
      break;
    default:
      ++len;
    }
  }
  return len + 1;
}

/*
 * Return an "inspected" version of the string.
 */

static const char *inspect(const char *str) {
  int j = 0;
  int len = inspect_length(str);
  char *buf = malloc(len);
  for (int i = 0; str[i]; ++i) {
    switch (str[i]) {
    case '\a':
    case '\b':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
      buf[j++] = '\\';
      buf[j++] = escapes[str[i] - 7];
      break;
    case '\e':
      buf[j++] = '\\';
      buf[j++] = 'e';
      break;
    default:
      buf[j++] = str[i];
    }
  }
  buf[j] = 0;
  return buf;
}

/*
 * Visit block `node`.
 */

static void visit_block(ifj17_visitor_t *self, ifj17_block_node_t *node) {
  ifj17_vec_each(node->stmts, {
    if (i)
      print_func("\n");
    INDENT;
    visit((ifj17_node_t *)val->value.as_pointer);
    if (!indents)
      print_func("\n");
  });
}

/*
 * Visit int `node`.
 */

static void visit_int(ifj17_visitor_t *self, ifj17_int_node_t *node) {
  print_func("(int %d)", node->val);
}

/*
 * Visit double `node`.
 */

static void visit_double(ifj17_visitor_t *self, ifj17_double_node_t *node) {
  print_func("(double %f)", node->val);
}

/*
 * Visit id `node`.
 */

static void visit_id(ifj17_visitor_t *self, ifj17_id_node_t *node) {
  print_func("(id %s)", node->val);
}

/*
 * Visit decl `node`.
 */

static void visit_decl(ifj17_visitor_t *self, ifj17_decl_node_t *node) {
  print_func("(decl");
  indents++;
  ifj17_vec_each(node->vec, {
    print_func("\n");
    INDENT;
    visit((ifj17_node_t *)val->value.as_pointer);
  });

  if (node->type) {
    print_func("\n");
    INDENT;
    print_func(": ");
    visit(node->type);
  }

  print_func(")");
  indents--;
}

/*
 * Visit dim `node`.
 */

static void visit_dim(ifj17_visitor_t *self, ifj17_dim_node_t *node) {
  print_func("(dim");
  indents++;

  ifj17_vec_each(node->vec, {
    ifj17_binary_op_node_t *bin = (ifj17_binary_op_node_t *)val->value.as_pointer;

    print_func("\n");
    INDENT;
    visit(bin->left);

    if (bin->right) {
      print_func("\n");
      INDENT;
      print_func(" = ");
      visit(bin->right);
    }
  });

  print_func(")");
  indents--;
}

/*
 * Visit string `node`.
 */

static void visit_string(ifj17_visitor_t *self, ifj17_string_node_t *node) {
  print_func("(string '%s')", inspect(node->val));
}

/*
 * Visit unary op `node`.
 */

static void visit_unary_op(ifj17_visitor_t *self, ifj17_unary_op_node_t *node) {
  print_func("(");

  if (node->postfix) {
    visit(node->expr);
    print_func(" %s", ifj17_token_type_string(node->op));
  } else {
    print_func("%s ", ifj17_token_type_string(node->op));
    visit(node->expr);
  }

  print_func(")");
}

/*
 * Visit binary op `node`.
 */

static void visit_binary_op(ifj17_visitor_t *self, ifj17_binary_op_node_t *node) {
  if (node->let) {
    print_func("(let %s ", ifj17_token_type_string(node->op));
  } else {
    print_func("(%s ", ifj17_token_type_string(node->op));
  }
  visit(node->left);
  print_func(" ");
  visit(node->right);
  print_func(")");
}

/*
 * Visit array `node`.
 */

static void visit_array(ifj17_visitor_t *self, ifj17_array_node_t *node) {
  print_func("(array\n");
  ++indents;
  ifj17_vec_each(node->vals, {
    INDENT;
    visit((ifj17_node_t *)val->value.as_pointer);
    if (i != len - 1)
      print_func("\n");
  });
  --indents;
  print_func(")");
}

/*
 * Visit hash `node`.
 */

static void visit_hash(ifj17_visitor_t *self, ifj17_hash_node_t *node) {
  print_func("(hash\n");
  ++indents;
  ifj17_vec_each(node->pairs, {
    INDENT;
    visit(((ifj17_hash_pair_node_t *)val->value.as_pointer)->key);
    print_func(": ");
    visit(((ifj17_hash_pair_node_t *)val->value.as_pointer)->val);
    print_func("\n");
  });
  --indents;
  print_func(")");
}

/*
 * Visit subscript `node`.
 */

static void visit_subscript(ifj17_visitor_t *self, ifj17_subscript_node_t *node) {
  print_func("(subscript\n");
  ++indents;
  INDENT;
  visit(node->left);
  print_func("\n");
  INDENT;
  visit(node->right);
  --indents;
  print_func(")");
}

/*
 * Visit slot `node`.
 */

static void visit_slot(ifj17_visitor_t *self, ifj17_slot_node_t *node) {
  print_func("(slot\n");
  ++indents;
  INDENT;
  visit(node->left);
  print_func("\n");
  INDENT;
  visit(node->right);
  --indents;
  print_func(")");
}

/*
 * Visit call `node`.
 */

static void visit_call(ifj17_visitor_t *self, ifj17_call_node_t *node) {
  print_func("(call\n");
  ++indents;
  INDENT;
  visit((ifj17_node_t *)node->expr);
  if (ifj17_vec_length(node->args->vec)) {
    print_func("\n");
    INDENT;
    ifj17_vec_each(node->args->vec, {
      visit((ifj17_node_t *)val->value.as_pointer);
      if (i != len - 1)
        print_func(" ");
    });

    ifj17_hash_each(node->args->hash, {
      print_func(" %s: ", slot);
      visit((ifj17_node_t *)val->value.as_pointer);
    });
  }
  --indents;
  print_func(")");
}

/*
 * Visit scope `scope`.
 */

static void visit_scope(ifj17_visitor_t *self, ifj17_scope_node_t *node) {
  print_func("(scope ->");
  ++indents;
  print_func("\n");
  visit((ifj17_node_t *)node->block);
  --indents;
  print_func(")");
}

static void visit_declare(ifj17_visitor_t *self, ifj17_declare_node_t *node) {
  print_func("(declare");
  print_func("\n");
  ++indents;
  INDENT;
  print_func("(function %s -> ", node->name);

  if (node->type) {
    visit(node->type);
  }

  ifj17_vec_each(node->params, {
    print_func("\n");
    ++indents;
    INDENT;
    visit((ifj17_node_t *)val->value.as_pointer);
    --indents;
  });
  --indents;
  print_func("))");
}

/*
 * Visit function `node`.
 */

static void visit_function(ifj17_visitor_t *self, ifj17_function_node_t *node) {
  print_func("(function %s -> ", node->name);
  ++indents;

  if (node->type) {
    visit(node->type);
  }

  ifj17_vec_each(node->params, {
    print_func("\n");
    INDENT;
    visit((ifj17_node_t *)val->value.as_pointer);
  });
  --indents;
  print_func("\n");
  ++indents;
  visit((ifj17_node_t *)node->block);
  --indents;
  print_func(")");
}

/*
 * Visit print `node`.
 */

static void visit_print(ifj17_visitor_t *self, ifj17_print_node_t *node) {
  print_func("(print");
  ++indents;

  ifj17_vec_each(node->params, {
    print_func("\n");
    INDENT;
    visit((ifj17_node_t *)val->value.as_pointer);
  });
  --indents;
  print_func(")\n");
}

/*
 * Visit `type` node.
 */

static void visit_type(ifj17_visitor_t *self, ifj17_type_node_t *node) {
  print_func("(type %s", node->name);
  ++indents;
  ifj17_vec_each(node->fields, {
    print_func("\n");
    INDENT;
    visit((ifj17_node_t *)val->value.as_pointer);
  });
  --indents;
  print_func(")");
}

/*
 * Visit `while` node.
 */

static void visit_while(ifj17_visitor_t *self, ifj17_while_node_t *node) {
  // while
  print_func("(while ");
  visit((ifj17_node_t *)node->expr);
  ++indents;
  print_func("\n");
  visit((ifj17_node_t *)node->block);
  --indents;
  print_func(")");
}

/*
 * Visit `return` node.
 */

static void visit_return(ifj17_visitor_t *self, ifj17_return_node_t *node) {
  print_func("(return");
  if (node->expr) {
    ++indents;
    print_func("\n");
    INDENT;
    visit((ifj17_node_t *)node->expr);
    --indents;
  }
  print_func(")");
}

/*
 * Visit if `node`.
 */

static void visit_if(ifj17_visitor_t *self, ifj17_if_node_t *node) {
  // if
  print_func("(if ");
  visit((ifj17_node_t *)node->expr);
  ++indents;
  print_func("\n");
  visit((ifj17_node_t *)node->block);
  --indents;
  print_func(")");

  // else ifs
  ifj17_vec_each(node->else_ifs, {
    ifj17_if_node_t *else_if = (ifj17_if_node_t *)val->value.as_pointer;
    print_func("\n");
    INDENT;
    print_func("(else if ");
    visit((ifj17_node_t *)else_if->expr);
    ++indents;
    print_func("\n");
    visit((ifj17_node_t *)else_if->block);
    --indents;
    print_func(")");
  });

  // else
  if (node->else_block) {
    print_func("\n");
    INDENT;
    print_func("(else\n");
    ++indents;
    visit((ifj17_node_t *)node->else_block);
    --indents;
    print_func(")");
  }
}

/*
 * Pretty-print the given `node` to stdout.
 */

void ifj17_prettyprint(ifj17_node_t *node) {
  ifj17_visitor_t visitor = {.visit_if = visit_if,
                             .visit_id = visit_id,
                             .visit_int = visit_int,
                             .visit_slot = visit_slot,
                             .visit_call = visit_call,
                             .visit_hash = visit_hash,
                             .visit_array = visit_array,
                             .visit_while = visit_while,
                             .visit_block = visit_block,
                             .visit_decl = visit_decl,
                             .visit_dim = visit_dim,
                             .visit_double = visit_double,
                             .visit_string = visit_string,
                             .visit_return = visit_return,
                             .visit_declare = visit_declare,
                             .visit_function = visit_function,
                             .visit_scope = visit_scope,
                             .visit_print = visit_print,
                             .visit_unary_op = visit_unary_op,
                             .visit_binary_op = visit_binary_op,
                             .visit_subscript = visit_subscript,
                             .visit_type = visit_type};

  ifj17_visit(&visitor, node);

  print_func("\n");
}
