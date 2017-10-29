//
// lexer.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_LEXER_H
#define IFJ17_LEXER_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "token.h"

#ifndef IFJ17_BUF_SIZE
#define IFJ17_BUF_SIZE 1024
#endif

/*
 * Lexer struct.
 */

typedef struct {
  char *error;
  int stash;
  int lineno;
  off_t offset;
  char *source;
  const char *filename;
  ifj17_token_t tok;
  char buf[IFJ17_BUF_SIZE];
} ifj17_lexer_t;

// protos

int
ifj17_scan(ifj17_lexer_t *self);

void
ifj17_lexer_init(ifj17_lexer_t *self, char *source, const char *filename);

#endif /* IFJ17_LEXER_H */
