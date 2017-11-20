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

 #define IFJ17_TOKEN_LIST   \
   t(ILLEGAL, "illegal")    \
   t(EOS, "end-of-source")  \
   t(ID, "id")              \
   t(INTEGER, "integer")    \
   t(DOUBLE, "double")      \
   t(CHR, "chr")            \
   t(STRING, "string")      \
   t(AS, "as")              \
   t(ASC, "asc")            \
   t(DECLARE, "declare")    \
   t(DIM, "dim")            \
   t(DO, "do")              \
   t(IF, "if")              \
   t(ELSE, "else")          \
   t(END, "end")            \
   t(FUNCTION, "function")  \
   t(INPUT, "input")        \
   t(LENGTH, "length")      \
   t(LOOP, "loop")          \
   t(PRINT, "print")        \
   t(RETURN, "return")      \
   t(SCOPE, "scope")        \
   t(SUBSTR, "substr")      \
   t(THEN, "then")          \
   t(WHILE, "while")        \
   t(AND, "and")            \
   t(BOOLEAN, "boolean")    \
   t(CONTINUE, "continue")  \
   t(ELSEIF, "elseif")      \
   t(EXIT, "exit")          \
   t(FALSE, "false")        \
   t(FOR, "for")            \
   t(NEXT, "next")          \
   t(OP_LNOT, "not")        \
   t(OR, "or")              \
   t(SHARED, "shared")      \
   t(STATIC, "static")      \
   t(TRUE, "true")          \
   t(LINECOMM, "'")         \
   t(LPAREN, "(")           \
   t(RPAREN, ")")           \
   t(SEMICOLON, ";")        \
   t(COMMA, ",")            \
   t(OP_DOT, ".")           \
   t(OP_MUL, "*")           \
   t(OP_DIV_DOUBLE, "/")    \
   t(OP_DIV_INTEGER, "\\")  \
   t(OP_PLUS, "+")          \
   t(OP_MINUS, "-")         \
   t(OP_ASSIGN, "=")        \
   t(OP_NOT_EQ, "<>")       \
   t(OP_GT, ">")            \
   t(OP_LT, "<")            \
   t(OP_GTE, ">=")          \
   t(OP_LTE, "<=")          \
/*
 * Tokens enum.
 */

typedef enum
{
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

typedef struct
{
  int len;
  ifj17_token type;
  struct
  {
    char *as_string;
    double as_double;
    int as_int;
  } value;
} ifj17_token_t;

/*
 * Return the string associated with the
 * given token `type`.
 */

static inline const char *ifj17_token_type_string(ifj17_token type) {
  assert(type <= IFJ17_TOKEN_OP_LTE);
  return ifj17_token_strings[type];
}

// protos
void ifj17_token_inspect(ifj17_token_t *tok);

#endif /* IFJ17_TOKEN_H */
