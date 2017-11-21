//
// parser.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_PARSER_H
#define IFJ17_PARSER_H

#include "lexer.h"
#include "ast.h"

/*
 * Parser struct.
 */

typedef struct {
  char *ctx;
  char *err;
  int in_args;
  ifj17_token_t *tok;
  ifj17_lexer_t *lex;
} ifj17_parser_t;

// prototypes

void
ifj17_parser_init(ifj17_parser_t *self, ifj17_lexer_t *lex);

ifj17_block_node_t *
ifj17_parse(ifj17_parser_t *self);

#endif /* IFJ17_PARSER_H */
