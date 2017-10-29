//
// token.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "token.h"
#include <stdio.h>

/*
 * Inspect the given `tok`, outputting debugging
 * information to stdout.
 */

void ifj17_token_inspect(ifj17_token_t *tok) {
  printf("\e[90m%s\e[0m", ifj17_token_type_string(tok->type));
  switch (tok->type) {
  case IFJ17_TOKEN_INT:
    printf(" \e[36m%d\e[0m", tok->value.as_int);
    break;
  case IFJ17_TOKEN_FLOAT:
    printf(" \e[36m%f\e[0m", tok->value.as_float);
    break;
  case IFJ17_TOKEN_STRING:
    printf(" \e[32m'%s'\e[0m", tok->value.as_string);
    break;
  case IFJ17_TOKEN_ID:
    printf(" %s", tok->value.as_string);
    break;
  }
  printf("\n");
}
