
#include <stdio.h>

typedef enum{

    // Key words
    AS,                       //0
    DECLARE,                  //1
    DO,                       //2
    INPUT,                    //3
    LOOP,                     //4
    PRINT,                    //5
    RETURN,                   //6
    SCOPE,                    //7
    EXIT,                     //8
    NEXT,                     //9
    SHARED,                   //10

    //Data types key-words
    STRING,                   //11
    INTEGER,                  //12
    DOUBLE,                   //13
    STATIC,                   //14

    // Functions key-words
    FUNCTION,                 //15
    LENGTH,                   //16
    SUBSTR,                   //17
    ASC,                      //18
    CHR,                      //19

    // Logic key-words
    BOOLEAN,                  //20
    TRUE,                     //21
    FALSE,                    //22
    AND,                      //23
    NOT,                      //24
    OR,                       //25

    // Cycles key-words
    WHILE,                    //26
    FOR,                      //27

    //Сonditional key-words
    IF,                       //28
    THEN,                     //29
    ELSEEND,                  //30
    ELSEIF,                   //31
    CONTINUE,                 //32

    // Operators
    ADD,                      //33
    MUL,                      //34
    SUB,                      //35
    DIV,                      //36
    LESS,                     //37
    GREATER,                  //38
    EQAL,                     //39
    NOT_EQUAL,                //40
    LESS_OR_EQUAL,            //41
    GREATER_OR_EQUAL,         //42

    // Identifiers
    ID,                       //43

    // Statement(s)
    DIM,                      //44

    // Literals
    INT,                      //45
    DOUB,                     //46
    DEC,                      //47
    STR_LIT,                  //48
    TERM,                     //49

    //Separators
    SPACE,                    //50
    TAB,                      //51
    NEW_LINE,                 //52
    LEFT_BRACKET,             //53
    RIGHT_BRACKET,            //54
    DOT,                      //55
    COMMA,                    //56
    SEMICOLON,                //57

    //Comments
    LINE_COMMENT,             //58
    BLOCK_COMMENT             //59

    INIT                      //60

    WORD                      //61
}Token_type;


/*typedef struct{
  int int_val;
	double double_val;
	char *str_val;
	bool bool_val;
}Token_value;*/

typedef struct{
  char *value;
  int value_index;
}Token_value;

typedef struct{
  Token_type type;
  Token_value value;
  /*int line_number;*/
}Token;

const N = 10;

Token *token_init();
/*Token_value set_token_val();*/
Token *get_next_token();
Token_Type key_word(Token *token);
