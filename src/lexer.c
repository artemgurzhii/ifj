//
// lexer.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "lexer.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

/*
 * Next char in the array.
 */

#ifdef EBUG_LEXER
#define next                                                                        \
  (self->stash = self->source[self->offset++],                                      \
   fprintf(stderr, "'%c'\n", self->stash), self->stash)
#else
#define next (self->stash = self->source[self->offset++])
#endif

/*
 * Undo the previous char.
 */

#define undo (self->source[--self->offset] = self->stash)

/*
 * Assign token `t`.
 */

#define token(t) (self->tok.type = IFJ17_TOKEN_##t)

/*
 * Accept char `c` or undo and return 0.
 */

#define accept(c) (c == next ? c : (undo, 0))

/*
 * Set error `msg` and assign ILLEGAL token.
 */

#define error(msg) (self->error = msg, token(ILLEGAL))

/*
 * True if the lexer should insert a semicolon after `t`.
 */

#define need_semi(t)                                                                \
  (t == IFJ17_TOKEN_ID || t == IFJ17_TOKEN_DOUBLE || t == IFJ17_TOKEN_INTEGER ||    \
   t == IFJ17_TOKEN_STRING || t == IFJ17_TOKEN_RETURN) // ????

/*
 * Initialize lexer with the given `source` and `filename`.
 */

void ifj17_lexer_init(ifj17_lexer_t *self, char *source, const char *filename) {
  self->error = NULL;
  self->source = source;
  self->filename = filename;
  self->lineno = 1;
  self->offset = 0;
}

/*
 * Convert hex digit `c` to a base 10 int,
 * return -1 when fails to convert.
 */

static int hex(const char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

/*
 * Scan identifier.
 */

static int scan_ident(ifj17_lexer_t *self, int c) {
  int len = 0;
  char buf[128]; // TODO: ditch these buffers
  token(ID);

  do {
    buf[len++] = c;
  } while (isalpha(c = next) || isdigit(c) || '_' == c);
  undo;

  buf[len++] = 0;
  switch (len - 1) {
  case 2:
    if (strcmp("if", buf) == 0)
      return token(IF);
    if (strcmp("as", buf) == 0)
      return token(AS);
    if (strcmp("do", buf) == 0)
      return token(DO);
    if (strcmp("or", buf) == 0)
      return token(OR);
    break;
  case 3:
    if (strcmp("asc", buf) == 0)
      return token(ASC);
    if (strcmp("dim", buf) == 0)
      return token(DIM);
    if (strcmp("end", buf) == 0)
      return token(END);
    if (strcmp("for", buf) == 0)
      return token(FOR);
    if (strcmp("and", buf) == 0)
      return token(AND);
    if (strcmp("not", buf) == 0)
      return token(OP_LNOT);
    break;
  case 4:
    if (strcmp("else", buf) == 0)
      return token(ELSE);
    if (strcmp("loop", buf) == 0)
      return token(LOOP);
    if (strcmp("exit", buf) == 0)
      return token(EXIT);
    if (strcmp("next", buf) == 0)
      return token(NEXT);
    if (strcmp("true", buf) == 0)
      return token(TRUE);
    break;
  case 5:
    if (strcmp("while", buf) == 0)
      return token(WHILE);
    if (strcmp("print", buf) == 0)
      return token(PRINT);
    if (strcmp("scope", buf) == 0)
      return token(SCOPE);
    if (strcmp("false", buf) == 0)
      return token(FALSE);
    break;
  case 6:
    if (strcmp("length", buf) == 0)
      return token(LENGTH);
    if (strcmp("substr", buf) == 0)
      return token(SUBSTR);
    if (strcmp("elseif", buf) == 0)
      return token(ELSEIF);
    if (strcmp("shared", buf) == 0)
      return token(SHARED);
    if (strcmp("static", buf) == 0)
      return token(STATIC);
    if (strcmp("return", buf) == 0)
      return token(RETURN);
    break;
  case 7:
    if (strcmp("then", buf) == 0)
      return token(THEN);
    break;
  case 8:
    if (strcmp("function", buf) == 0)
      return token(FUNCTION);
    if (strcmp("continue", buf) == 0)
      return token(CONTINUE);
    break;
  default:
    break;
  }

  self->tok.value.as_string = strdup(buf);
  return 1;
}

/*
 * Scan string hex literal, returning -1 on invalid digits.
 */

static int hex_literal(ifj17_lexer_t *self) {
  int a = hex(next);
  int b = hex(next);
  if (a > -1 && b > -1)
    return a << 4 | b;
  error("string hex literal \\x contains invalid digits");
  return -1;
}

/*
 * Scan string.
 */
// TODO delete quote arg
static int scan_string(ifj17_lexer_t *self, int quote) {
  int c, len = 0;
  char buf[128]; // TODO: change to realloc
  token(STRING);

  while (quote != (c = next)) {
    switch (c) {
    case '\n':
      ++self->lineno;
      break;
    case '\\':
      switch (c = next) {
      case 'a':
        c = '\a';
        break;
      case 'b':
        c = '\b';
        break;
      case 'e':
        c = '\e';
        break;
      case 'f':
        c = '\f';
        break;
      case 'n':
        c = '\n';
        break;
      case 'r':
        c = '\r';
        break;
      case 't':
        c = '\t';
        break;
      case 'v':
        c = '\v';
        break;
      case 'x':
        if ((c = hex_literal(self)) == -1)
          return 0;
      }
      break;
    }
    buf[len++] = c;
  }

  buf[len++] = 0;
  self->tok.value.as_string = strdup(buf); // TODO: remove
  return 1;
}

/*
 * Scan number.
 */

static int scan_number(ifj17_lexer_t *self, int c) {
  int n = 0, type = 0, expo = 0, e;
  int expo_type = 1;
  /* expo_type:
   * 1 -> '+'(default)
   * 0 -> '-'
   */
  token(INTEGER);

  switch (c) {
  case '0':
    goto scan_hex;
  default:
    goto scan_int;
  }

scan_hex:
  switch (c = next) {
  case 'x':
    if (!isxdigit(c = next)) {
      error("hex literal expects one or more digits");
      return 0;
    } else {
      do
        n = n << 4 | hex(c);
      while (isxdigit(c = next));
    }
    self->tok.value.as_int = n;
    undo;
    return 1;
  default:
    undo;
    c = '0';
    goto scan_int;
  }

// [0-9_]+

scan_int:
  do {
    if ('_' == c)
      continue;
    else if ('.' == c)
      goto scan_double;
    else if ('e' == c || 'E' == c)
      goto scan_expo;
    n = n * 10 + c - '0';
  } while (isdigit(c = next) || '_' == c || '.' == c || 'e' == c || 'E' == c);
  undo;
  self->tok.value.as_int = n;
  return 1;

// [0-9_]+

scan_double : {
  e = 1;
  type = 1;
  token(DOUBLE);
  while (isdigit(c = next) || '_' == c || 'e' == c || 'E' == c) {
    if ('_' == c)
      continue;
    else if ('e' == c || 'E' == c)
      goto scan_expo;
    n = n * 10 + c - '0';
    e *= 10;
  }
  undo;
  self->tok.value.as_double = (double)n / e;
  return 1;
}

// [\+\-]?[0-9]+

scan_expo : {
  while (isdigit(c = next) || '+' == c || '-' == c) {
    if ('-' == c) {
      expo_type = 0;
      continue;
    }
    expo = expo * 10 + c - '0';
  }

  undo;
  if (expo_type == 0)
    expo *= -1;
  if (type == 0)
    self->tok.value.as_int = n * pow(10, expo);
  else
    self->tok.value.as_double = ((double)n / e) * pow(10, expo);
}

  return 1;
}

/*
 * Scan the next token in the stream, returns 0
 * on EOS, ILLEGAL token, or a syntax error.
 */

int ifj17_scan(ifj17_lexer_t *self) {
  int c;

// scan
scan:
  switch (c = next) {
  case ' ':
  case '\t':
    goto scan;
  case '(':
    return token(LPAREN);
  case ')':
    return token(RPAREN);
  case ',':
    return token(COMMA);
  case '.':
    return token(OP_DOT);
  case '+':
    return token(OP_PLUS);
  case '-':
    return token(OP_MINUS);
  case '*':
    return token(OP_MUL);
  case '/':
    return token(OP_DIV_DOUBLE);
  case '\\':
    return token(OP_DIV_INTEGER);
  case '!':
    return '"' == next ? scan_string(self, c) : token(ILLEGAL);
  case '=':
    return token(OP_ASSIGN);
  case '<':
    switch (next) {
    case '=':
      return token(OP_LTE);
    case '>':
      return token(OP_NOT_EQ);
    default:
      return undo, token(OP_LT);
    }
  case '>':
    switch (next) {
    case '=':
      return token(OP_GTE);
    default:
      return undo, token(OP_GT);
    }
  case ';':
    return token(SEMICOLON);
  case '\n':
  case '\r':
    if (need_semi(self->tok.type)) {
      return undo, token(SEMICOLON);
    }
    ++self->lineno;
    goto scan; // TODO: Check if need_semi is needed
  case 0:
    token(EOS);
    return 0;
  default:
    if (isalpha(c) || '_' == c)
      return scan_ident(self, c);
    if (isdigit(c) || '.' == c)
      return scan_number(self, c);
    token(ILLEGAL);
    error("illegal character");
    return 0;
  }
}
