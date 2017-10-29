//
// prettyprint.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_PP_H
#define IFJ17_PP_H

#include "ast.h"

void
ifj17_set_prettyprint_func(int (*func)(const char *format, ...));

void
ifj17_prettyprint(ifj17_node_t *node);

#endif /* IFJ17_PP_H */
