//
// vec.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "vec.h"
#include "internal.h"

/*
 * Alloc and initialize a new array.
 */

ifj17_vec_t *ifj17_vec_new() {
  ifj17_vec_t *self = malloc(sizeof(ifj17_vec_t));

  if (unlikely(!self)) {
    return NULL;
  }

  ifj17_vec_init(self);

  return self;
}
