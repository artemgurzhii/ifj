#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "token.h"
#include "token_functions.c"

Token *tokinizer(FILE *source){

  bool start_msg = FALSE;// "
  bool start_block_comment = FALSE;// /'
  bool end_block_comment = FALSE;//  '/

  int current_token = 0; // index for token array
  int realoc_val = N; // var for value realloc
  int realoc_tok = N; // var for token realloc

  int chr;
  Token *token_array = (Token *)malloc(sizeof(Token)*N);
  Token *token = token_array[current_token];
  token_init(token);

  while(chr = fgetc(source)){

    if(token->value->value_index >= N){// if token value is longer than we thought
      realoc_val += N;
      token->value->value = (char*)realloc(token->value, sizeof(char)*realoc_val);
    }
    if(current_token >= N){/*if our programm have more
                           tokens then we thought*/
      realoc_tok += N;
      token_array = (Token*)realloc(token_array, sizeof(Token)*realoc_tok);
    }

    switch (token->type) {
      case INIT: // New token
        if(chr == '+') token->type = ADD;
        else if(chr == '-') token->type = SUB;
        else if(chr == '*') token->type = MUL;
        else if(chr == '%') token->type = DIV;
        else if(chr == '=') token->type = EQAL;
        else if(chr == '\n') token->type = NEW_LINE;
        else if(chr == '\t') token->type = TAB;
        else if(chr == ' ') token->type = SPACE;
        else if(chr == '(') token->type = LEFT_BRACKET;
        else if(chr == ')') token->type = RIGHT_BRACKET;
        else if(chr == '.') token->type = DOT
        else if(chr == ',') token->type = COMMA;
        else if(chr == ';') token->type = SEMICOLON;
        else if(chr == '!') token->type = STR;
        else if(chr == '<') token->type = LESS;
        else if(chr == '>') token->type = GREATER;
        else if(chr == '39') token->type = LINE_COMMENT;
        else if(chr == '/') token->type = BLOCK_COMMENT;
        else if(isdigit(chr)){
          token->value[token->value->value_index++] = chr;
          token->type = INT;
        }
        else if(isalpha(chr)){
          token->value[token->value->value_index++] = chr;
          token->type = WORD;
        }

        /*Move to next token if we have complete current one*/
        if(token->type == ADD ||
           token->type == SUB ||
           token->type == MUL ||
           token->type == DIV ||
           token->type == EQAL ||
           token->type == NEW_LINE ||
           token->type == TAB ||
           token->type == SPACE ||
           token->type == LEFT_BRACKET ||
           token->type == RIGHT_BRACKET ||
           token->type == DOT ||
           token->type == COMMA ||
           token->type == SEMICOLON ||){
             token = token_array[++current_token];
             token_init(token);
        }
      break;


      /*If we have <, >, [0-9], [a-z], [A-Z], ', /, ! as our last token*/
      case LESS:
        if(chr == '=') token->type = LESS_OR_EQUAL;
        else if(chr == '>') token->type = NOT_EQUAL;
        token = token_array[++current_token]; // move to next token
        token_init(token);
      break;

      case GREATER:
        if(chr == '=') token->type = GREATER_OR_EQUAL;
        token = token_array[++current_token]; // move to next token
        token_init(token);
      break;

      case INT:
        if(chr == '.') {
          token->value[token->value->value_index++] = chr;
          token->type = DOUB;
        }
        else if(isdigit(chr)) token->value[token->value->value_index++] = chr;
        else if(chr == ' '){
          token = token_array[++current_token];
          token->type = SPACE;
          token = token_array[++current_token]; // adding SPACE and move to next token
          token_init(token);
        }
      break;

      case DOUB:
        if(isdigit(chr)) token->value[token->value->value_index++] = chr;
        else if(chr == ' '){
          token = token_array[++current_token];
          token->type = SPACE;
          token = token_array[++current_token]; // adding SPACE and move to next token
          token_init(token);
        }
      break;

      case WORD:
        if(isalpha(chr) || isdigit(chr)) token->value[token->value->value_index++] = chr;
        else if(chr == ' '){
          token->type = key_word(token); // set token->type
          token = token_array[++current_token];
          token->type = SPACE;
          token = token_array[++current_token];// adding SPACE and move to next token
          token_init(token);
        }
      break;

      case LINE_COMMENT:
        if(chr == '\n') {
          token = token_array[++current_token];
          token->type = NEW_LINE;
          token = token_array[++current_token];// adding NEW_LINE and move to next token
          token_init(token);
        }
        else token->value[token->value->value_index++] = chr;
      break;

      case BLOCK_COMMENT: // '/'
        if (chr == '39'/* ' */ && !start_block_comment) {
          start_block_comment = TRUE;
          continue;
        }
        else if (chr == '39' && start_block_comment){// checking if we have '
          end_block_comment = TRUE;
          continue;
        }
        else if(chr == '/' && start_block_comment && end_block_comment){ // end of comment
          token = token_array[++current_token];// move to next token
          token_init(token);
          start_block_comment = FALSE;
          end_block_comment = FALSE;
        }
        else if(start_block_comment) {
          token->value[token->value->value_index++] = chr;
          end_block_comment = FALSE;
        }
      break;

      case STR: // '!'
        if(chr == '34' /* " */ && !start_msg){
          start_msg = TRUE;
          continue;
        }
        else if(chr == '34' && start_msg){ // end of string
          token = token_array[++current_token]; // move to next token
          token_init(token);
          start_msg = FALSE;
        }
        else token->value[token->value->value_index++] = chr;
      break;
    }
  }
  return *token_array;
}
