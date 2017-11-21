//
// state.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_STATE_H
#define IFJ17_STATE_H

#include "khash.h"

// TODO: move

typedef struct {
  int len;
  char *val;
} ifj17_string_t;

KHASH_MAP_INIT_STR(str, ifj17_string_t *);

/*
 * IFJ17 state.
 */

typedef struct {
  khash_t(str) * strs;
} ifj17_state_t;

// prototypes

void ifj17_state_init(ifj17_state_t *self);

// TODO: move

ifj17_string_t *ifj17_string(ifj17_state_t *state, char *val);

#endif /* IFJ17_STATE_H */
