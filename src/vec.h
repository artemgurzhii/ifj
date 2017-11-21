//
// vec.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_VEC_H
#define IFJ17_VEC_H

#include "kvec.h"
#include "object.h"

/*
 * IFJ17 array.
 */

typedef kvec_t(ifj17_object_t *) ifj17_vec_t;

/*
 * Initialize an array.
 */

#define ifj17_vec_init(self) kv_init(*self)

/*
 * Return the array length.
 */

#define ifj17_vec_length(self) kv_size(*self)

/*
 * Push `obj` into the array.
 */

#define ifj17_vec_push(self, obj) kv_push(ifj17_object_t *, *self, obj)

/*
 * Pop an object out of the array.
 */

#define ifj17_vec_pop(self) (ifj17_vec_length(self) ? kv_pop(*self) : NULL)

/*
 * Return the object at `i`.
 */

#define ifj17_vec_at(self, i)                                                       \
  (((i) >= 0 && (i) < ifj17_vec_length(self)) ? kv_A(*self, (i)) : NULL)

/*
 * Iterate the array, populating `i` and `val`.
 */

#define ifj17_vec_each(self, block)                                                 \
  {                                                                                 \
    ifj17_object_t *val;                                                            \
    int len = ifj17_vec_length(self);                                               \
    for (int i = 0; i < len; ++i) {                                                 \
      val = ifj17_vec_at(self, i);                                                  \
      block;                                                                        \
    }                                                                               \
  }

// prototypes

ifj17_vec_t *ifj17_vec_new();

#endif /* IFJ17_VEC_H */
