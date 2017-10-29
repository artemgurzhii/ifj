//
// state.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "state.h"

/*
 * Initialize ifj17 state:
 *
 *   - initialize string vector
 */

void ifj17_state_init(ifj17_state_t *self) { self->strs = kh_init(str); }
