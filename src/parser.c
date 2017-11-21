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

// -EBUG_PARSER

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

static ifj17_block_node_t *block(ifj17_parser_t *self);
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
  if (accept(LPAREN) == false) {
    return NULL;
  }
  if ((node = expr(self)) == false) {
    return NULL;
  }
  if (accept(RPAREN) == false) {
    return error("expression missing closing ')'");
  }
  return node;
}

/*
 * id
 */

static ifj17_node_t *type_expr(ifj17_parser_t *self) {
  debug("type_expr");
  if (is(ID) == false) {
    return NULL;
  }

  ifj17_node_t *ret =
      (ifj17_node_t *)ifj17_id_node_new(self->tok->value.as_string, lineno);
  next;

  return ret;
}

/*
 * Dim id as type_expr
 */
// TODO: remove need_type, var decl in ifj17 always requires type (compare with luna)
// TODO: not shure if works as required, need to be tested

static ifj17_node_t *decl_expr(ifj17_parser_t *self, bool need_type) {
  debug("decl_expr");
  context("declaration");

  ifj17_vec_t *vec = ifj17_vec_new();
  int decl_line = lineno;

  // 'dim'
  if (accept(DIM) == false) {
    return error("expecting dim");
  }

  next;

  if (is(ID) == false) {
    return error("expecting id");
  }

  // id
  ifj17_node_t *id =
      (ifj17_node_t *)ifj17_id_node_new(self->tok->value.as_string, lineno);
  ifj17_vec_push(vec, ifj17_node(id));
  next;

  // 'as'
  if (accept(AS) == false) {
    return error("expecting as");
  }

  next;

  ifj17_node_t *type = type_expr(self);
  if (type == false) {
    return error("expecting type");
  }

  return (ifj17_node_t *)ifj17_decl_node_new(vec, type, decl_line);
}

/*
 *   id
 * | int
 * | double
 * | string
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
  case IFJ17_TOKEN_INTEGER:
    ret = (ifj17_node_t *)ifj17_int_node_new(tok->value.as_int, lineno);
    break;
  case IFJ17_TOKEN_DOUBLE:
    ret = (ifj17_node_t *)ifj17_double_node_new(tok->value.as_double, lineno);
    break;
  case IFJ17_TOKEN_STRING:
    ret = (ifj17_node_t *)ifj17_string_node_new(tok->value.as_string, lineno);
    break;
  }
  if (ret) {
    next;
    return ret;
  }
  return paren_expr(self);
}

/*
 * | '+' unary_expr
 * | '-' unary_expr
 * | '!' unary_expr
 * | primary_expr
 */ // TODO: there are 3 unary ops in ifj17 - +,-,'not'. Need to make it work with
    // 'not'

static ifj17_node_t *unary_expr(ifj17_parser_t *self) {
  debug("unary_expr");
  ifj17_node_t *node;
  int line = lineno;
  if (is(OP_PLUS) || is(OP_MINUS)) {
    node = (ifj17_node_t *)ifj17_unary_op_node_new(self->tok->type, unary_expr(self),
                                                   0, line);
    next;
  }
  return node;
}

/*
 * unary_expr (('* | '/' | '\') unary_expr)*
 */

static ifj17_node_t *multiplicative_expr(ifj17_parser_t *self) {
  ifj17_token op;
  ifj17_node_t *node, *right;
  int line = lineno;
  debug("multiplicative_expr");
  if ((node = unary_expr(self)) == false) {
    return NULL;
  }
  while (is(OP_MUL) || is(OP_DIV_DOUBLE) || is(OP_DIV_INTEGER)) {
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
  if ((node = multiplicative_expr(self)) == false) {
    return NULL;
  }
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
 * additive_expr (('<' | '<=' | '>' | '>=') additive_expr)*
 */

static ifj17_node_t *relational_expr(ifj17_parser_t *self) {
  ifj17_token op;
  ifj17_node_t *node, *right;
  int line = lineno;
  debug("relational_expr");
  if ((node = additive_expr(self)) == false) {
    return NULL;
  }
  while (is(OP_LT) || is(OP_LTE) || is(OP_GT) || is(OP_GTE)) {
    op = self->tok->type;
    next;
    context("relational operation");
    if (right = additive_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(op, node, right, line);
    } else {
      return error("missing right-hand expression");
    }
  }
  return node;
}

/*
 * relational_expr (('==' | '<>') relational_expr)*
 */
// TODO: ifj17 doesn't have '==' - lol, but let it be here for now

static ifj17_node_t *equality_expr(ifj17_parser_t *self) {
  ifj17_token op;
  ifj17_node_t *node, *right;
  int line = lineno;
  debug("equality_expr");
  if ((node = relational_expr(self)) == false) {
    return NULL;
  }
  while (is(OP_NOT_EQ)) {
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
 * equality_expr ('&&' equality_expr)*
 */

static ifj17_node_t *logical_and_expr(ifj17_parser_t *self) {
  ifj17_node_t *node, *right;
  debug("logical_and_expr");
  int line = lineno;
  if ((node = equality_expr(self)) == false) {
    return NULL;
  }
  while (accept(AND)) {
    context("&& operation");
    if (right = equality_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(IFJ17_TOKEN_AND, node, right,
                                                      line);
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
  if ((node = logical_and_expr(self)) == false) {
    return NULL;
  }

  // '||'
  while (accept(OR)) {
    context("|| operation");
    if (right = logical_and_expr(self)) {
      node = (ifj17_node_t *)ifj17_binary_op_node_new(IFJ17_TOKEN_OR, node, right,
                                                      line);
    } else {
      return error("missing right-hand expression");
    }
  }

  return node;
}

/*
 * (decl_expr ('=' expr)? (',' decl_expr ('=' expr)?)*)
 */
// TODO: Check if feature is needed

static ifj17_vec_t *function_params(ifj17_parser_t *self) {
  ifj17_vec_t *params = ifj17_vec_new();
  debug("params");
  context("function params");

  if (is(ID) == false) {
    return params;
  }

  do {
    int line = lineno;
    ifj17_node_t *decl = decl_expr(self, false);
    if (decl == false) {
      return NULL;
    }

    context("function param");

    // ('=' expr)?
    ifj17_object_t *param;
    if (accept(OP_ASSIGN)) {
      ifj17_node_t *val = expr(self);
      if (val == false) {
        return NULL;
      }
      param = ifj17_node((ifj17_node_t *)ifj17_binary_op_node_new(
          IFJ17_TOKEN_OP_ASSIGN, decl, val, line));
    } else {
      // if there isn't a value we need a type
      if (decl->type == false) {
        return error("expecting type");
      }
      param = ifj17_node((ifj17_node_t *)decl);
    }

    ifj17_vec_push(params, param);
  } while (accept(COMMA));

  return params;
}

/*
 *  params? block
 */

static ifj17_node_t *function_expr(ifj17_parser_t *self) {
  ifj17_block_node_t *body;
  ifj17_vec_t *params;
  debug("function_expr");

  // params?
  if ((params = function_params(self)) == false) {
    return NULL;
  }

  context("function");

  // block
  if (body = block(self)) {
    // return (ifj17_node_t *) ifj17_function_node_new(body, params);
    // TODO: line 457, see how it works and why commented in luna
  }

  return NULL;
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
    if ((node = expr(self)) == false) {
      return NULL;
    }
    ifj17_vec_push(args->vec, ifj17_node(node));
  } while (accept(COMMA));

  self->in_args--;

  return args;
}

/*
 *   logical_or_expr
 * | let_expr
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

  debug("assignment_expr");
  if ((node = logical_or_expr(self)) == false) {
    return NULL;
  }

  // =
  if (is(OP_ASSIGN)) {
    op = self->tok->type;
    next;
    context("assignment");
    if ((right = not_expr(self)) == false) {
      return NULL;
    }
    ifj17_binary_op_node_t *ret = ifj17_binary_op_node_new(op, node, right, line);
    return (ifj17_node_t *)ret;
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
    if ((expr = not_expr(self)) == false) {
      return NULL;
    }
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
  if ((node = not_expr(self)) == false) {
    return NULL;
  }
  return node;
}

/*
 * 'function' id '(' args? ')' ('as' type_expr)? block
 */

static ifj17_node_t *function_stmt(ifj17_parser_t *self) {
  ifj17_block_node_t *body;
  ifj17_vec_t *params;
  ifj17_node_t *type = NULL;
  int line = lineno;
  debug("function_stmt");
  context("function statement");

  // 'function'
  if (accept(FUNCTION) == false) {
    return NULL;
  }

  // id
  if (is(ID) == false) {
    return error("missing function name");
  }

  const char *name = self->tok->value.as_string;
  next;

  // '('
  if (accept(LPAREN)) {
    // params?
    if ((params = function_params(self)) == false) {
      return NULL;
    }
    // ')'
    context("function");
    if (accept(RPAREN) == false) {
      return error("missing closing ')'");
    }
  } else {
    params = ifj17_vec_new();
  }

  context("function");

  // ('AS' type_expr)?
  if (accept(AS) == false) {
    type = type_expr(self);
    if (type == false) {
      return error("missing type after ':'");
    }
  }

  // block
  if (body = block(self)) {
    return (ifj17_node_t *)ifj17_function_node_new(name, type, body, params, line);
  }

  return NULL;
}

/*
 *  'if' expr block
 *  ('else' 'if' block)*
 *  ('else' block)?
 */
// TODO: remove negate, fix ELSEIF
static ifj17_node_t *if_stmt(ifj17_parser_t *self) {
  ifj17_node_t *cond;
  ifj17_block_node_t *body;
  int line = lineno;
  debug("if_stmt");

  // if
  if (is(IF) == false) {
    return NULL;
  }
  int negate = 0;
  next;

  // expr
  context("if statement condition");
  if ((cond = expr(self)) == false) {
    return NULL;
  }

  accept(THEN);

  // block
  context("if statement");
  if ((body = block(self)) == false) {
    return NULL;
  }

  ifj17_if_node_t *node = ifj17_if_node_new(negate, cond, body, line);

// 'else'
loop:
  if (accept(ELSE)) {
    ifj17_block_node_t *body;

    // ('else' 'if' block)*
    if (accept(IF)) {
      int line = lineno;
      context("else if statement condition");
      if ((cond = expr(self)) == false) {
        return NULL;
      }
      // semicolon might have been inserted here
      accept(SEMICOLON);

      context("else if statement");
      if ((body = block(self)) == false) {
        return NULL;
      }
      ifj17_vec_push(node->else_ifs, ifj17_node((ifj17_node_t *)ifj17_if_node_new(
                                         0, cond, body, line)));
      goto loop;
      // 'else'
    } else {
      context("else statement");
      if ((body = block(self)) == false) {
        return NULL;
      }
      node->else_block = body;
    }
  }

  return (ifj17_node_t *)node;
}

/*
 * 'while' expr block
 */
// TODO: remove negate
static ifj17_node_t *while_stmt(ifj17_parser_t *self) {
  ifj17_node_t *cond;
  ifj17_block_node_t *body;
  int line = lineno;
  debug("while_stmt");

  // ('while')
  if (is(WHILE) == false) {
    return NULL;
  }
  int negate = 0;
  context("while statement condition");
  next;

  // expr
  if ((cond = expr(self)) == false) {
    return NULL;
  }
  context("while statement");

  // block
  if ((body = block(self)) == false) {
    return NULL;
  }

  return (ifj17_node_t *)ifj17_while_node_new(negate, cond, body, line);
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
  if (accept(RETURN) == false) {
    return NULL;
  }
  // 'return' expr
  ifj17_node_t *node = NULL;

  if (accept(SEMICOLON) == false) {
    if ((node = expr(self)) == false) {
      return NULL;
    }
  }
  return (ifj17_node_t *)ifj17_return_node_new(node, line);
}

/*
 *   if_stmt
 * | while_stmt
 * | return_stmt
 * | function_stmt
 * | expr
 */

static ifj17_node_t *stmt(ifj17_parser_t *self) {
  debug("stmt");
  context("statement");
  if (is(IF)) {
    return if_stmt(self);
  }
  if (is(WHILE)) {
    return while_stmt(self);
  }
  if (is(RETURN)) {
    return return_stmt(self);
  }
  if (is(FUNCTION)) {
    return function_stmt(self);
  }
  return expr(self);
}

/*
 * stmt* 'end'
 */
// TODO: add fuction end, scope end, etc.
static ifj17_block_node_t *block(ifj17_parser_t *self) {
  debug("block");
  ifj17_node_t *node;
  ifj17_block_node_t *block = ifj17_block_node_new(lineno);

  if (accept(END)) {
    return block;
  }

  do {
    if ((node = stmt(self)) == false) {
      return NULL;
    }

    accept(SEMICOLON);

    ifj17_vec_push(block->stmts, ifj17_node(node));
  } while ((accept(END) == false) && (is(ELSE) == false));

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
