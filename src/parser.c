//
// parser.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "parser.h"
#include "prettyprint.h"
#include "token.h"
#include "vec.h"
#include <stdbool.h>
#include <stdio.h>

// TODO: test contextual errors

// -DEBUG_PARSER

#ifdef EBUG_PARSER
#define debug(name)                                                                 \
  fprintf(stderr, "\n\e[90m%s\e[0m\n", name);                                       \
  ifj17_token_inspect(&self->lex->tok);
#else
#define debug(name)
#endif

/*
 * Advance if the current token is `t`
 */

#ifdef EBUG_PARSER
#define accept(t)                                                                   \
  (is(t)) \
    ? (fprintf(stderr, "\e[90maccepted \e[33m%s\e[0m\n", #t), next, 1) \
    : 0)
#else
#define accept(t) (is(t) && next)
#endif

/*
 * Consume a token from the lexer
 */

#define next (self->tok = (ifj17_scan(self->lex), &self->lex->tok))

/*
 * Check if the current token is `t`.
 */

#define is(t) (self->tok->type == IFJ17_TOKEN_##t)

/*
 * Set error context `str`, used in error reporting.
 */

#define context(str) (self->ctx = str)

/*
 * Set error `str` when not previously set.
 */

#define error(str) ((self->err = self->err ? self->err : str), NULL)

/*
 * The lexer's current line number.
 */

#define lineno self->lex->lineno

// forward declarations

static ifj17_block_node_t *block(ifj17_parser_t *self, bool isFunctionBlock);
static ifj17_node_t *expr(ifj17_parser_t *self);
static ifj17_node_t *call_expr(ifj17_parser_t *self, ifj17_node_t *left);
static ifj17_node_t *not_expr(ifj17_parser_t *self);

/*
 * Initialize with the given lexer.
 */

void ifj17_parser_init(ifj17_parser_t *self, ifj17_lexer_t *lex) {
  self->lex = lex;
  self->tok = NULL;
  self->ctx = NULL;
  self->err = NULL;
  self->in_args = 0;
}

/*
 * '(' expr ')'
 */

static ifj17_node_t *paren_expr(ifj17_parser_t *self) {
  ifj17_node_t *node;
  debug("paren_expr");

  if (!accept(LPAREN)) {
    return NULL;
  }

  if (!(node = expr(self))) {
    return NULL;
  }

  if (!accept(RPAREN)) {
    return error("expression missing closing ')'");
  }

  return node;
}

/*
 *   expr ','?
 * | expr ',' arg_list
 */

int arg_list(ifj17_parser_t *self, ifj17_array_node_t *arr, ifj17_token delim) {
  // trailing ','
  if (delim == self->tok->type)
    return 1;

  // expr
  ifj17_node_t *val;
  if (!(val = expr(self)))
    return 0;

  ifj17_vec_push(arr->vals, ifj17_node(val));

  // ',' arg_list
  if (accept(COMMA)) {
    if (!arg_list(self, arr, delim))
      return 0;
  }

  return 1;
}

/*
 * '[' arg_list? ']'
 */

static ifj17_node_t *array_expr(ifj17_parser_t *self) {
  ifj17_array_node_t *node = ifj17_array_node_new(lineno);
  debug("array_expr");

  if (!accept(LBRACK))
    return NULL;
  context("array");
  if (!arg_list(self, node, IFJ17_TOKEN_RBRACK))
    return NULL;
  if (!accept(RBRACK))
    return error("array missing closing ']'");
  return (ifj17_node_t *)node;
}

/*
 *   id ':' expr
 * | id ':' expr ',' hash_pairs
 */

int hash_pairs(ifj17_parser_t *self, ifj17_hash_node_t *hash, ifj17_token delim) {
  // trailing ','
  if (delim == self->tok->type)
    return 1;

  ifj17_hash_pair_node_t *pair = ifj17_hash_pair_node_new(lineno);
  if (!(pair->key = expr(self)))
    return 0;

  // 'as'
  if (!accept(AS)) {
    return error("hash pair ':' missing"), 0;
  }

  // expr
  if (!(pair->val = expr(self)))
    return 0;

  ifj17_vec_push(hash->pairs, ifj17_node((ifj17_node_t *)pair));

  // ',' hash_pairs
  if (accept(COMMA)) {
    if (!hash_pairs(self, hash, delim))
      return 0;
  }

  return 1;
}

/*
 * '{' hash_pairs? '}'
 */

static ifj17_node_t *hash_expr(ifj17_parser_t *self) {
  ifj17_hash_node_t *node = ifj17_hash_node_new(lineno);
  debug("hash_expr");

  if (!accept(LBRACE))
    return NULL;
  context("hash");
  if (!hash_pairs(self, node, IFJ17_TOKEN_RBRACE))
    return NULL;
  if (!accept(RBRACE))
    return error("hash missing closing '}'");
  return (ifj17_node_t *)node;
}

/*
 * id
 */

static ifj17_node_t *type_expr(ifj17_parser_t *self) {
  debug("type_expr");
  if (!is(ID))
    return NULL;

  ifj17_node_t *ret =
      (ifj17_node_t *)ifj17_id_node_new(self->tok->value.as_string, lineno);
  next;

  return ret;
}

/*
 * id (',' id)* ':' type_expr
 */

static ifj17_node_t *decl_expr(ifj17_parser_t *self, bool need_type) {
  debug("decl_expr");
  context("declaration");

  if (!is(ID)) {
    return NULL;
  }

  ifj17_vec_t *vec = ifj17_vec_new();
  int decl_line = lineno;
  while (is(ID)) {
    // id
    ifj17_node_t *id =
        (ifj17_node_t *)ifj17_id_node_new(self->tok->value.as_string, lineno);
    ifj17_vec_push(vec, ifj17_node(id));
    next;

    // ','
    if (!accept(COMMA)) {
      break;
    }
  }

  // 'as'
  if (!accept(AS)) {
    if (need_type) {
      return error("expecting type");
    } else {
      return (ifj17_node_t *)ifj17_decl_node_new(vec, NULL, decl_line);
    }
  }

  ifj17_node_t *type = type_expr(self);
  if (!type) {
    return error("expecting type");
  }

  return (ifj17_node_t *)ifj17_decl_node_new(vec, type, decl_line);
}

/*
 *   id
 * | int
 * | float
 * | string
 * | array
 * | hash
 * | paren_expr
 */

static ifj17_node_t *primary_expr(ifj17_parser_t *self) {
  debug("primary_expr");
  ifj17_node_t *ret = NULL;
  ifj17_token_t *tok = self->tok;
  switch (tok->type) {
  case IFJ17_TOKEN_ID:
    ret = (ifj17_node_t *)ifj17_id_node_new(tok->value.as_string, lineno);
    break;
  case IFJ17_TOKEN_INT:
    ret = (ifj17_node_t *)ifj17_int_node_new(tok->value.as_int, lineno);
    break;
  case IFJ17_TOKEN_DOUBLE:
    ret = (ifj17_node_t *)ifj17_double_node_new(tok->value.as_double, lineno);
    break;
  case IFJ17_TOKEN_STRING:
    ret = (ifj17_node_t *)ifj17_string_node_new(tok->value.as_string, lineno);
    break;
  case IFJ17_TOKEN_LBRACK:
    return array_expr(self);
  case IFJ17_TOKEN_LBRACE:
    return hash_expr(self);
  }
  if (ret) {
    next;
    return ret;
  }
  return paren_expr(self);
}

/*
 *   call_expr
 * | call_expr '**' call_expr
 */

static ifj17_node_t *pow_expr(ifj17_parser_t *self) {
  ifj17_node_t *node, *right;
  int line = lineno;

  debug("pow_expr");
  if (!(node = call_expr(self, NULL)))
    return NULL;
  if (accept(OP_POW)) {
    context("** operation");
    if (right = call_expr(self, NULL)) {
      return (ifj17_node_t *)ifj17_binary_op_node_new(IFJ17_TOKEN_OP_POW, node,
                                                      right, line);
    } else {
      return error("missing right-hand expression");
    }
  }
  return node;
}

/*
 *   pow_expr
 * | pow_expr '++'
 * | pow_expr '--'
 */

static ifj17_node_t *postfix_expr(ifj17_parser_t *self) {
  ifj17_node_t *node;
  int line = lineno;
  debug("postfix_expr");
  if (!(node = pow_expr(self)))
    return NULL;
  if (is(OP_INCR) || is(OP_DECR)) {
    node = (ifj17_node_t *)ifj17_unary_op_node_new(self->tok->type, node, 1, line);
    next;
  }
  return node;
}

/*
 *   '++' unary_expr
 * | '--' unary_expr
 * | '~' unary_expr
 * | '+' unary_expr
 * | '-' unary_expr
 * | '!' unary_expr
 * | primary_expr
 */

static ifj17_node_t *unary_expr(ifj17_parser_t *self) {
  debug("unary_expr");
  int line = lineno;
  if (is(OP_INCR) || is(OP_DECR) || is(OP_BIT_NOT) || is(OP_PLUS) || is(OP_MINUS) ||
      is(OP_NOT)) {
    int op = self->tok->type;
    next;
    return (ifj17_node_t *)ifj17_unary_op_node_new(op, unary_expr(self), 0, line);
  }
  return postfix_expr(self);
}

/*
 * unary_expr (('* | '/' | '%') unary_expr)*
 */

static ifj17_node_t *multiplicative_expr(ifj17_parser_t *self) {
  ifj17_token op;
  ifj17_node_t *node, *right;
  int line = lineno;
  debug("multiplicative_expr");
  if (!(node = unary_expr(self)))
    return NULL;
  while (is(OP_MUL) || is(OP_DIV) || is(OP_MOD)) {
    op = self->tok->type;
    next;
    context("multiplicative operation");
    if (right = unary_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(op, node, right, line);
    } else {
      return error("missing right-hand expression");
    }
  }
  return node;
}

/*
 * multiplicative_expr (('+ | '-') multiplicative_expr)*
 */

static ifj17_node_t *additive_expr(ifj17_parser_t *self) {
  ifj17_token op;
  ifj17_node_t *node, *right;
  int line = lineno;
  debug("additive_expr");
  if (!(node = multiplicative_expr(self)))
    return NULL;
  while (is(OP_PLUS) || is(OP_MINUS)) {
    op = self->tok->type;
    next;
    context("additive operation");
    if (right = multiplicative_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(op, node, right, line);
    } else {
      return error("missing right-hand expression");
    }
  }
  return node;
}

/*
 * additive_expr (('<<' | '>>') additive_expr)*
 */

static ifj17_node_t *shift_expr(ifj17_parser_t *self) {
  ifj17_token op;
  ifj17_node_t *node, *right;
  int line = lineno;
  debug("shift_expr");
  if (!(node = additive_expr(self)))
    return NULL;
  while (is(OP_BIT_SHL) || is(OP_BIT_SHR)) {
    op = self->tok->type;
    next;
    context("shift operation");
    if (right = additive_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(op, node, right, line);
    } else {
      return error("missing right-hand expression");
    }
  }
  return node;
}

/*
 * shift_expr (('<' | '<=' | '>' | '>=') shift_expr)*
 */

static ifj17_node_t *relational_expr(ifj17_parser_t *self) {
  ifj17_token op;
  ifj17_node_t *node, *right;
  int line = lineno;
  debug("relational_expr");
  if (!(node = shift_expr(self)))
    return NULL;
  while (is(OP_LT) || is(OP_LTE) || is(OP_GT) || is(OP_GTE)) {
    op = self->tok->type;
    next;
    context("relational operation");
    if (right = shift_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(op, node, right, line);
    } else {
      return error("missing right-hand expression");
    }
  }
  return node;
}

/*
 * relational_expr (('==' | '!=') relational_expr)*
 */

static ifj17_node_t *equality_expr(ifj17_parser_t *self) {
  ifj17_token op;
  ifj17_node_t *node, *right;
  int line = lineno;
  debug("equality_expr");
  if (!(node = relational_expr(self)))
    return NULL;
  while (is(OP_EQ) || is(OP_NEQ)) {
    op = self->tok->type;
    next;
    context("equality operation");
    if (right = relational_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(op, node, right, line);
    } else {
      return error("missing right-hand expression");
    }
  }
  return node;
}

/*
 * equality_expr ('and' equality_expr)*
 */

static ifj17_node_t *bitwise_and_expr(ifj17_parser_t *self) {
  ifj17_node_t *node, *right;
  debug("bitwise_and_expr");
  int line = lineno;
  if (!(node = equality_expr(self)))
    return NULL;
  while (accept(OP_BIT_AND)) {
    context("& operation");
    if (right = equality_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(IFJ17_TOKEN_OP_BIT_AND, node,
                                                      right, line);
    } else {
      return error("missing right-hand expression");
    }
  }
  return node;
}

/*
 * bitwise_and_expr ('^' bitwise_and_expr)*
 */

static ifj17_node_t *bitwise_xor_expr(ifj17_parser_t *self) {
  ifj17_node_t *node, *right;
  debug("bitwise_xor_expr");
  int line = lineno;
  if (!(node = bitwise_and_expr(self)))
    return NULL;
  while (accept(OP_BIT_XOR)) {
    context("^ operation");
    if (right = bitwise_and_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(IFJ17_TOKEN_OP_BIT_XOR, node,
                                                      right, line);
    } else {
      return error("missing right-hand expression");
    }
  }
  return node;
}

/*
 * bitwise_xor_expr ('|' bitwise_xor_expr)*
 */

static ifj17_node_t *bitswise_or_expr(ifj17_parser_t *self) {
  ifj17_node_t *node, *right;
  debug("bitswise_or_expr");
  int line = lineno;
  if (!(node = bitwise_xor_expr(self)))
    return NULL;
  while (accept(OP_BIT_OR)) {
    context("| operation");
    if (right = bitwise_xor_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(IFJ17_TOKEN_OP_BIT_OR, node,
                                                      right, line);
    } else {
      return error("missing right-hand expression");
    }
  }
  return node;
}

/*
 * bitswise_or_expr ('&&' bitswise_or_expr)*
 */

static ifj17_node_t *logical_and_expr(ifj17_parser_t *self) {
  ifj17_node_t *node, *right;
  debug("logical_and_expr");
  int line = lineno;
  if (!(node = bitswise_or_expr(self)))
    return NULL;
  while (accept(OP_AND)) {
    context("&& operation");
    if (right = bitswise_or_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(IFJ17_TOKEN_OP_AND, node,
                                                      right, line);
    } else {
      return error("missing right-hand expression");
    }
  }
  return node;
}

/*
 * logical_and_expr ('||' logical_and_expr)* '&'?
 */

static ifj17_node_t *logical_or_expr(ifj17_parser_t *self) {
  ifj17_node_t *node, *right;
  int line = lineno;
  debug("logical_or_expr");
  if (!(node = logical_and_expr(self)))
    return NULL;

  // '||'
  while (accept(OP_OR)) {
    context("|| operation");
    if (right = logical_and_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(IFJ17_TOKEN_OP_OR, node, right,
                                                      line);
    } else {
      return error("missing right-hand expression");
    }
  }

  // '&'
  if (accept(OP_FORK)) {
    ifj17_id_node_t *id = ifj17_id_node_new("fork", line);
    ifj17_call_node_t *call = ifj17_call_node_new((ifj17_node_t *)id, line);
    ifj17_vec_push(call->args->vec, ifj17_node(node));
    node = (ifj17_node_t *)call;
  }

  return node;
}

/*
 * (decl_expr ('=' expr)? (',' decl_expr ('=' expr)?)*)
 */

static ifj17_vec_t *function_params(ifj17_parser_t *self) {
  ifj17_vec_t *params = ifj17_vec_new();
  debug("params");
  context("function params");

  if (!is(ID))
    return params;

  do {
    int line = lineno;
    ifj17_node_t *decl = decl_expr(self, false);
    if (!decl)
      return NULL;

    context("function param");

    // ('=' expr)?
    ifj17_object_t *param;
    if (accept(OP_ASSIGN)) {
      ifj17_node_t *val = expr(self);
      if (!val)
        return NULL;
      param = ifj17_node((ifj17_node_t *)ifj17_binary_op_node_new(
          IFJ17_TOKEN_OP_ASSIGN, decl, val, line));
    } else {
      // if there isn't a value we need a type
      if (!decl->type) {
        return error("expecting type");
      }
      param = ifj17_node((ifj17_node_t *)decl);
    }

    ifj17_vec_push(params, param);
  } while (accept(COMMA));

  return params;
}

/*
 * ':' params? block
 */

static ifj17_node_t *function_expr(ifj17_parser_t *self) {
  ifj17_block_node_t *body;
  ifj17_vec_t *params;
  debug("function_expr");

  // 'as'
  if (accept(AS)) {
    // params?
    if (!(params = function_params(self))) {
      return NULL;
    }

    context("function");

    // block
    if (body = block(self, false)) {
      // return (ifj17_node_t *) ifj17_function_node_new(body, params);
    }
  }

  return NULL;
}

/*
 *   primary_expr
 * | primary_expr '[' expr ']'
 * | primary_expr '.' id
 * | primary_expr '.' call_expr
 */

static ifj17_node_t *slot_access_expr(ifj17_parser_t *self, ifj17_node_t *left) {
  int line = lineno;
  debug("slot_access_expr");

  // primary_expr
  if (!left) {
    if (!(left = primary_expr(self)))
      return NULL;
  }

  // subscript
  if (accept(LBRACK)) {
    ifj17_node_t *right;

    if (!(right = expr(self))) {
      return error("missing index in subscript");
    }
    context("subscript");
    if (!accept(RBRACK))
      return error("missing closing ']'");
    left = (ifj17_node_t *)ifj17_subscript_node_new(left, right, line);
    return call_expr(self, left);
  }

  // slot
  while (accept(OP_DOT)) {
    context("slot access");
    if (!is(ID))
      return error("expecting identifier");
    ifj17_node_t *id =
        (ifj17_node_t *)ifj17_id_node_new(self->tok->value.as_string, lineno);
    next;

    if (is(LPAREN)) {
      ifj17_call_node_t *call;
      ifj17_vec_t *args_vec;
      ifj17_object_t *prev = NULL;

      call = (ifj17_call_node_t *)call_expr(self, id);
      if (ifj17_vec_length(call->args->vec) > 0) {

        // re-organize call arguments (issue #47)
        args_vec = ifj17_vec_new();
        ifj17_vec_each(call->args->vec, {
          if (i == 0) {
            ifj17_vec_push(args_vec, ifj17_node(left));
          } else {
            ifj17_vec_push(args_vec, prev);
          }

          prev = val;
        });
      } else {
        prev = ifj17_node(left);
        args_vec = call->args->vec;
      }

      // add last argument
      ifj17_vec_push(args_vec, prev);

      // TODO: free the old arguments vector
      call->args->vec = args_vec;
      left = (ifj17_node_t *)call;

    } else {
      left = (ifj17_node_t *)ifj17_slot_node_new(left, id, line);
    }

    left = call_expr(self, left);
  }

  return left;
}

/*
 * (expr (',' expr)*)
 */

ifj17_args_node_t *call_args(ifj17_parser_t *self) {
  ifj17_node_t *node;
  ifj17_args_node_t *args = ifj17_args_node_new(lineno);

  self->in_args++;

  debug("args");
  do {
    if (node = expr(self)) {
      if (accept(AS)) {
        context("keyword argument");

        if (node->type != IFJ17_NODE_STRING && node->type != IFJ17_NODE_ID) {
          return error("expecting string or identifier as key");
        }

        ifj17_node_t *val = expr(self);
        const char *str = ((ifj17_id_node_t *)node)->val;
        ifj17_hash_set(args->hash, (char *)str, ifj17_node(val));
      } else {
        ifj17_vec_push(args->vec, ifj17_node(node));
      }
    } else {
      return NULL;
    }
  } while (accept(COMMA));

  self->in_args--;

  return args;
}

/*
 *   slot_access_expr '(' args? ')'
 * | slot_access_expr
 */

static ifj17_node_t *call_expr(ifj17_parser_t *self, ifj17_node_t *left) {
  ifj17_node_t *right;
  ifj17_node_t *prev = left;
  ifj17_call_node_t *call = NULL;
  int line = lineno;
  debug("call_expr");

  // slot_access_expr
  if (!left) {
    if (!(left = slot_access_expr(self, NULL)))
      return NULL;
  }

  // '('
  if (accept(LPAREN)) {
    context("function call");
    call = ifj17_call_node_new(left, line);

    // args? ')'
    if (!is(RPAREN)) {
      call->args = call_args(self);
      if (!is(RPAREN))
        return error("missing closing ')'");
    }
    next;
    left = (ifj17_node_t *)call;
  }

  if (is(OP_DOT) && prev) {
    // stop here if the there was a previous left-hand expression
    // and the current token is '.' because we're
    // probably inside the loop in slot_access_expr
    return left;
  } else if (is(LPAREN)) {
    return call_expr(self, left);
  } else {
    return slot_access_expr(self, left);
  }
}

/*
 * 'dim' decl_expr ('=' expr)? (',' decl_expr ('=' expr)?)*
 */

static ifj17_node_t *dim_expr(ifj17_parser_t *self) {
  // dim already consumed
  ifj17_vec_t *vec = ifj17_vec_new();
  int let_line = lineno;

  do {
    int line = lineno;
    ifj17_node_t *decl = decl_expr(self, false);
    ifj17_node_t *val = NULL;

    context("dim expression");
    if (!decl) {
      return error("expecting declaration");
    }

    // '='
    if (accept(OP_ASSIGN)) {
      val = expr(self);
      if (!val)
        return error("expecting declaration initializer");
    }

    ifj17_node_t *bin = (ifj17_node_t *)ifj17_binary_op_node_new(
        IFJ17_TOKEN_OP_ASSIGN, decl, val, line);
    ifj17_vec_push(vec, ifj17_node(bin));
  } while (accept(COMMA));

  return (ifj17_node_t *)ifj17_dim_node_new(vec, let_line);
}

/*
 *   logical_or_expr
 * | dim_expr
 * | call_expr '=' not_expr
 * | call_expr '+=' not_expr
 * | call_expr '-=' not_expr
 * | call_expr '/=' not_expr
 * | call_expr '*=' not_expr
 * | call_expr '||=' not_expr
 */

static ifj17_node_t *assignment_expr(ifj17_parser_t *self) {
  ifj17_token op;
  ifj17_node_t *node, *right;
  int line = lineno;

  // dim?
  if (accept(DIM)) {
    return dim_expr(self);
  }

  debug("assignment_expr");
  if (!(node = logical_or_expr(self)))
    return NULL;

  // =
  if (is(OP_ASSIGN)) {
    op = self->tok->type;
    next;
    context("assignment");
    if (!(right = not_expr(self)))
      return NULL;
    ifj17_binary_op_node_t *ret = ifj17_binary_op_node_new(op, node, right, line);
    return (ifj17_node_t *)ret;
  }

  // compound
  if (is(OP_PLUS_ASSIGN) || is(OP_MINUS_ASSIGN) || is(OP_DIV_ASSIGN) ||
      is(OP_MUL_ASSIGN) || is(OP_OR_ASSIGN) || is(OP_AND_ASSIGN)) {
    op = self->tok->type;
    next;
    context("compound assignment");
    if (!(right = not_expr(self)))
      return NULL;
    return (ifj17_node_t *)ifj17_binary_op_node_new(op, node, right, line);
  }

  return node;
}

/*
 *   'not' not_expr
 * | assignment_expr
 */

static ifj17_node_t *not_expr(ifj17_parser_t *self) {
  int line = lineno;
  debug("not_expr");
  if (accept(OP_LNOT)) {
    ifj17_node_t *expr;
    if (!(expr = not_expr(self)))
      return NULL;
    return (ifj17_node_t *)ifj17_unary_op_node_new(IFJ17_TOKEN_OP_LNOT, expr, 0,
                                                   line);
  }
  return assignment_expr(self);
}

/*
 *  not_expr
 */

static ifj17_node_t *expr(ifj17_parser_t *self) {
  ifj17_node_t *node;
  debug("expr");
  if (!(node = not_expr(self)))
    return NULL;
  return node;
}

/*
 * 'type' id decl_expr* end
 */

static ifj17_node_t *type_stmt(ifj17_parser_t *self) {
  debug("type_stmt");
  context("type statement");
  int line = lineno;

  // 'type'
  if (!accept(TYPE)) {
    return NULL;
  }

  // id
  if (!is(ID)) {
    return error("missing type name");
  }

  const char *name = self->tok->value.as_string;
  ifj17_type_node_t *type = ifj17_type_node_new(name, line);
  next;

  // semicolon might have been inserted here
  accept(SEMICOLON);

  // type fields
  do {
    ifj17_node_t *decl = decl_expr(self, true);
    if (!decl)
      return error("expecting field");

    // semicolon might have been inserted here
    accept(SEMICOLON);

    ifj17_vec_push(type->fields, ifj17_node(decl));
  } while (!accept(END));

  return (ifj17_node_t *)type;
}

/*
 * 'scope' expr block
 */

static ifj17_node_t *scope_stmt(ifj17_parser_t *self) {
  ifj17_block_node_t *body;
  int line = lineno;
  debug("scope_stmt");
  context("scope statement");

  // 'scope'
  if (!accept(SCOPE)) {
    return NULL;
  }

  // block
  if (body = block(self, false)) {
    return (ifj17_node_t *)ifj17_scope_node_new(body, line);
  }

  return NULL;
}

/*
 * Function declaration
 */

static ifj17_node_t *func_declare(ifj17_parser_t *self) {
  // declare already consumed
  ifj17_vec_t *params;
  ifj17_node_t *type = NULL;
  int line = lineno;

  debug("function_decl");

  if (!accept(DECLARE)) {
    return NULL;
  }

  context("function declaration");
  // 'function'
  if (!accept(FUNCTION)) {
    return error("expecting 'function'");
  }

  // id
  if (!is(ID)) {
    return error("missing function name");
  }

  const char *name = self->tok->value.as_string;
  next;

  // '('
  if (accept(LPAREN)) {
    // params?
    if (!(params = function_params(self)))
      return NULL;

    // ')'
    context("function declaration");
    if (!accept(RPAREN)) {
      return error("missing closing ')'");
    }
  } else {
    params = ifj17_vec_new();
  }

  context("function declaration");

  // (':' type_expr)?
  if (accept(AS)) {
    type = type_expr(self);

    if (!type) {
      return error("missing type after ':'");
    }
  }

  // semicolon might have been inserted here
  accept(SEMICOLON);

  return (ifj17_node_t *)ifj17_declare_node_new(name, type, params, line);
}

/*
 * 'function' id '(' args? ')' (':' type_expr)? block
 */

static ifj17_node_t *function_stmt(ifj17_parser_t *self) {
  ifj17_block_node_t *body;
  ifj17_vec_t *params;
  ifj17_node_t *type = NULL;
  int line = lineno;
  debug("function_stmt");
  context("function statement");

  // 'function'
  if (!accept(FUNCTION)) {
    return NULL;
  }

  // id
  if (!is(ID)) {
    return error("missing function name");
  }

  const char *name = self->tok->value.as_string;
  next;

  // '('
  if (accept(LPAREN)) {
    // params?
    if (!(params = function_params(self)))
      return NULL;

    // ')'
    context("function");
    if (!accept(RPAREN)) {
      return error("missing closing ')'");
    }
  } else {
    params = ifj17_vec_new();
  }

  context("function");

  // (':' type_expr)?
  if (accept(AS)) {
    type = type_expr(self);

    if (!type) {
      return error("missing type after ':'");
    }
  }

  // semicolon might have been inserted here
  accept(SEMICOLON);

  // block
  if (body = block(self, true)) {
    return (ifj17_node_t *)ifj17_function_node_new(name, type, body, params, line);
  }

  return NULL;
}

/*
 *  ('if') expr block
 *  ('elseif' block)*
 *  ('else' block)?
 */

static ifj17_node_t *if_stmt(ifj17_parser_t *self) {
  ifj17_node_t *cond;
  ifj17_block_node_t *body;
  int line = lineno;
  debug("if_stmt");

  // 'if'
  if (!is(IF)) {
    return NULL;
  }

  next;

  // expr
  context("if statement condition");
  if (!(cond = expr(self))) {
    return NULL;
  }

  if (!accept(THEN)) {
    return error("missing 'then'");
  }

  // block
  context("if statement");
  if (!(body = block(self, false))) {
    return NULL;
  }

  ifj17_if_node_t *node = ifj17_if_node_new(cond, body, line);

// 'elseif' || 'else'
loop:
  if (accept(ELSE)) {
    context("else statement");

    if (!(body = block(self, false))) {
      return NULL;
    }

    node->else_block = body;
  } else if (accept(ELSEIF)) {
    context("elseif statement condition");

    if (!(cond = expr(self))) {
      return NULL;
    }

    if (!accept(THEN)) {
      return error("missing 'then'");
    }

    // block
    context("if statement");
    if (!(body = block(self, false))) {
      return NULL;
    }

    ifj17_vec_push(node->else_ifs,
                   ifj17_node((ifj17_node_t *)ifj17_if_node_new(cond, body, line)));
    goto loop;
  }

  return (ifj17_node_t *)node;
}

/*
 * 'while' expr block
 */

static ifj17_node_t *while_stmt(ifj17_parser_t *self) {
  ifj17_node_t *cond;
  ifj17_block_node_t *body;
  int line = lineno;
  debug("do_while_stmt");

  // 'do'
  if (!accept(DO)) {
    return NULL;
  }

  if (!accept(WHILE)) {
    return error("missing 'while'");
  }

  context("while statement condition");

  // BUG: From what I have seen in our language docs
  // there is 2 ways of `do while` loop
  // 1. do while ${condition}
  // 1. do while ( ${condition} )
  // Need to find out which is correct or both of them

  // expr
  if (!(cond = expr(self))) {
    return NULL;
  }

  // semicolon might have been inserted here
  accept(SEMICOLON);

  // block
  if (!(body = block(self, false))) {
    return NULL;
  }

  return (ifj17_node_t *)ifj17_while_node_new(cond, body, line);
}

/*
 *   'return' expr
 * | 'return'
 */

static ifj17_node_t *return_stmt(ifj17_parser_t *self) {
  int line = lineno;
  debug("return");
  context("return statement");

  // 'return'
  if (!accept(RETURN)) {
    return NULL;
  }

  // 'return' expr
  ifj17_node_t *node = NULL;

  if (!accept(SEMICOLON)) {
    if (!(node = expr(self)))
      return NULL;
  }

  return (ifj17_node_t *)ifj17_return_node_new(node, line);
}

/*
 *   if_stmt
 * | while_stmt
 * | return_stmt
 * | scope_stmts
 * | function_stmt
 * | type_stmt
 * | expr
 */

static ifj17_node_t *stmt(ifj17_parser_t *self) {
  debug("stmt");
  context("statement");
  if (is(IF)) {
    return if_stmt(self);
  }

  if (is(DO)) {
    return while_stmt(self);
  }

  if (is(RETURN)) {
    return return_stmt(self);
  }

  if (is(SCOPE)) {
    return scope_stmt(self);
  }

  if (is(DECLARE)) {
    return func_declare(self);
  }

  if (is(FUNCTION)) {
    return function_stmt(self);
  }

  if (is(TYPE)) {
    return type_stmt(self);
  }

  return expr(self);
}

/*
 * stmt* 'end'
 */

static ifj17_block_node_t *block(ifj17_parser_t *self, bool isFunctionBlock) {
  debug("block");
  ifj17_node_t *node;
  ifj17_block_node_t *block = ifj17_block_node_new(lineno);

  // If `end` keyword is received
  // We need to check if this is a `function statement` or any other
  // As `function statement` requires `end function` for function closing

  if (accept(END) || accept(LOOP)) {

    // if (isFunctionBlock) {
    //   if (is(FUNCTION)) {
    //     return block;
    //   }
    // }
    //   return error("missing closing statement");
    // } else {
    return block;
    // }
  }

  do {
    if (!(node = stmt(self))) {
      return NULL;
    }

    // semicolon might have been inserted here
    accept(SEMICOLON);

    ifj17_vec_push(block->stmts, ifj17_node(node));
  } while ((!accept(END) && !is(ELSEIF) && !is(ELSE)) && (!accept(LOOP)));

  return block;
}

/*
 * stmt*
 */

static ifj17_block_node_t *program(ifj17_parser_t *self) {
  debug("program");
  ifj17_node_t *node;
  ifj17_block_node_t *block = ifj17_block_node_new(lineno);

  next;
  while (!is(EOS)) {
    if (node = stmt(self)) {
      accept(SEMICOLON);
      ifj17_vec_push(block->stmts, ifj17_node(node));
    } else {
      return NULL;
    }
  }

  return block;
}

/*
 * Parse input.
 */

ifj17_block_node_t *ifj17_parse(ifj17_parser_t *self) { return program(self); }
