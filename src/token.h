//
// token.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_TOKEN_H
#define IFJ17_TOKEN_H

#include <assert.h>

/*
 * Tokens.
 */

#define IFJ17_TOKEN_LIST \
  t(ILLEGAL, "illegal") \
  t(EOS, "end-of-source") \
  t(ID, "id") \
  t(INT, "int") \
  t(DOUBLE, "double") \
  t(STRING, "string") \
  t(FUNCTION, "function") \
  t(TYPE, "type") \
  t(AS, "as") \
  t(WHILE, "while") \
  t(IF, "if") \
  t(ELSE, "else") \
  t(FOR, "for") \
  t(DIM, "dim") \
  t(END, "end") \
  t(RETURN, "return") \
  t(LBRACE, "{") \
  t(RBRACE, "}") \
  t(LPAREN, "(") \
  t(RPAREN, ")") \
  t(LBRACK, "[") \
  t(RBRACK, "]") \
  t(COLON, ":") \
  t(QMARK, "?") \
  t(SEMICOLON, ";") \
  t(COMMA, ",") \
  t(OP_DOT, ".") \
  t(OP_LNOT, "not") \
  t(OP_NOT, "!") \
  t(OP_FORK, "&") \
  t(OP_PLUS, "+") \
  t(OP_INCR, "++") \
  t(OP_MINUS, "-") \
  t(OP_DECR, "--") \
  t(OP_MUL, "*") \
  t(OP_DIV, "/") \
  t(OP_MOD, "%") \
  t(OP_POW, "**") \
  t(OP_GT, ">") \
  t(OP_LT, "<") \
  t(OP_GTE, ">=") \
  t(OP_LTE, "<=") \
  t(OP_EQ, "==") \
  t(OP_NEQ, "!=") \
  t(OP_AND, "&&") \
  t(OP_OR, "||") \
  t(OP_ASSIGN, "=") \
  t(OP_PLUS_ASSIGN, "+=") \
  t(OP_MINUS_ASSIGN, "-=") \
  t(OP_MUL_ASSIGN, "*=") \
  t(OP_DIV_ASSIGN, "/=") \
  t(OP_AND_ASSIGN, "&&=") \
  t(OP_OR_ASSIGN, "||=") \
  t(OP_BIT_AND, "and") \
  t(OP_BIT_OR, "|") \
  t(OP_BIT_XOR, "^") \
  t(OP_BIT_NOT, "~") \
  t(OP_BIT_SHL, "<<") \
  t(OP_BIT_SHR, ">>")

/*
 * Tokens enum.
 */

typedef enum {
#define t(tok, str) IFJ17_TOKEN_##tok,
IFJ17_TOKEN_LIST
#undef t
} ifj17_token;

/*
 * Token strings.
 */

static char *ifj17_token_strings[] = {
#define t(tok, str) str,
IFJ17_TOKEN_LIST
#undef t
};

/*
 * Token struct.
 */

typedef struct {
  int len;
  ifj17_token type;
  struct {
    char *as_string;
    double as_double;
    int as_int;
  } value;
} ifj17_token_t;

/*
 * Return the string associated with the
 * given token `type`.
 */

static inline const char *
ifj17_token_type_string(ifj17_token type) {
  assert(type <= IFJ17_TOKEN_OP_BIT_SHR);
  return ifj17_token_strings[type];
}

// protos

void
ifj17_token_inspect(ifj17_token_t *tok);

#endif /* IFJ17_TOKEN_H */
