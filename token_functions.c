#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "token.h"


Token *token_init(Token *token){
  token->token_type = INIT;
  token->value->value = (char *)malloc(sizeof(char)*N);
  token->value->value_index = 0;
  return *token;
}

/*Token_value set_token_val(int *int_val, double *double_val, char **str_val, bool *bool_val, Token *token){
  token->value->int_val = int_val;
  token->value->double_val = double_val;
  token->value->str_val = *str_val;
  token->value->bool_val = bool_val;
}*/

Token *get_next_token(){

}

Token_Type key_word(Token *token){
  for (int i = 0; token->value[i]; i++) tolower(token->value[i]);
  if (token->value == "as") token->type = AS;
  else if (token->value == "declare") tokken->type = DECLARE;
  else if (token->value == "do") token->type = DO;
  else if (token->value == "input") token->type = INPUT;
  else if (token->value == "loop") token->type = LOOP;
  else if (token->value == "print") token->type = PRINT;
  else if (token->value == "return") token->type = RETURN;
  else if (token->value == "scope") token->type = SCOPE;
  else if (token->value == "exit") token->type = EXIT;
  else if (token->value == "next") token->type = NEXT;
  else if (token->value == "shared") token->type = SHARED;
  else if (token->value == "string") token->type = STRING;
  else if (token->value == "integer") token->type = INTEGER;
  else if (token->value == "double") token->type = DOUBLE;
  else if (token->value == "static") token->type = STATIC;
  else if (token->value == "function") token->type = FUNCTION;
  else if (token->value == "length") token->type = LENGTH;
  else if (token->value == "substr") token->type = SUBSTR;
  else if (token->value == "asc") token->type = ASC;
  else if (token->value == "chr") token->type = CHR;
  else if (token->value == "boolean") token->type = BOOLEAN;
  else if (token->value == "true") token->type = TRUE;
  else if (token->value == "false") token->type = FALSE;
  else if (token->value == "and") token->type = AND;
  else if (token->value == "not") token->type = NOT;
  else if (token->value == "or") token->type = OR;
  else if (token->value == "while") token->type = WHILE;
  else if (token->value == "for") token->type = FOR;
  else if (token->value == "if") token->type = IF;
  else if (token->value == "then") token->type = THEN;
  else if (token->value == "else") token->type = ELSE;
  else if (token->value == "end") token->type = END;
  else if (token->value == "continue") token->type = CONTINUE;
  else if (token->value == "dim") token->type = DIM;
  else token->type = ID;
  return token->type;
}
