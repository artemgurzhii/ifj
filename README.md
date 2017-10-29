[![Build Status](https://travis-ci.com/artemgurzhii/ifj.svg?token=y45aqd5TBgzdoqtyW6ox&branch=master)](https://travis-ci.com/artemgurzhii/ifj)

## Tools
[Tools and Frameworks](https://github.com/aalhour/awesome-compilers#c--c)

## Books
[Awesome Compilers books collection](https://github.com/aalhour/awesome-compilers#books)
Especially `The infamous Dragons Book, a classic textbook on Compiler Construction.`


## Description
### Errors
Error code
 - 1 - *Lexical Analysis Error*(takes the raw code and splits it apart into these things called tokens by a thing called a tokenizer (or lexer))
 - 2 - *Syntactic Analysis Error*(takes the tokens and reformats them into a representation that describes each part of the syntax and their relation to one another. This is known as an intermediate representation or Abstract Syntax Tree(AST))
 - 3 - *Semantic Error* - undefined *variable*, *function* etc, attempt to redefine *variable*, *function* etc.
 - 4 - *Type Error* - unexpected/incorrect variable type passed.
 - 6 - Other semantic errors.
 - 99
