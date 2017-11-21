//
// errors.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov
// Evgeny
//

#include "errors.h"

/*
 * Report syntax or parse error.
 */

void ifj17_report_error(ifj17_parser_t *parser) {
  char *err, *type = "parse";
  ifj17_lexer_t *lex = parser->lex;

  // error message
  if (parser->err) {
    err = parser->err;
    // lexer
  } else if (lex->error) {
    err = lex->error;
    type = "syntax";
    // generate
  } else {
    char buf[64];
    snprintf(buf, 64, "unexpected token '%s'",
             ifj17_token_type_string(lex->tok.type));
    err = buf;
  }

  fprintf(stderr, "ifj17(%s:%d). %s error in %s, %s.\n", lex->filename, lex->lineno,
          type, parser->ctx, err);
}
