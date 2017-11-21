//
// hash.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_HASH_H
#define IFJ17_HASH_H

#include "khash.h"

// ifj17 object

typedef struct ifj17_object_struct ifj17_object_t;

// value hash

KHASH_MAP_INIT_STR(value, ifj17_object_t *);

/*
 * IFJ17 hash.
 */

typedef khash_t(value) ifj17_hash_t;

/*
 * Allocate a new hash.
 */

#define ifj17_hash_new() kh_init(value)

/*
 * Destroy the hash.
 */

#define ifj17_hash_destroy(self) kh_destroy(value, self)

/*
 * Hash size.
 */

#define ifj17_hash_size kh_size

/*
 * Iterate hash slots and values, populating
 * `slot` and `val`.
 */

#define ifj17_hash_each(self, block) { \
   const char *slot; \
   ifj17_object_t *val; \
    for (khiter_t k = kh_begin(self); k < kh_end(self); ++k) { \
      if (!kh_exist(self, k)) continue; \
      slot = kh_key(self, k); \
      val = kh_value(self, k); \
      block; \
    } \
  }

/*
 * Iterate hash slots, populating `slot`.
 */

#define ifj17_hash_each_slot(self, block) { \
    const char *slot; \
    for (khiter_t k = kh_begin(self); k < kh_end(self); ++k) { \
      if (!kh_exist(self, k)) continue; \
      slot = kh_key(self, k); \
      block; \
    } \
  }

/*
 * Iterate hash values, populating `val`.
 */

#define ifj17_hash_each_val(self, block) { \
    ifj17_object_t *val; \
    for (khiter_t k = kh_begin(self); k < kh_end(self); ++k) { \
      if (!kh_exist(self, k)) continue; \
      val = kh_value(self, k); \
      block; \
    } \
  }

// prototypes

void
ifj17_hash_set(khash_t(value) *self, char *key, ifj17_object_t *val);

ifj17_object_t *
ifj17_hash_get(khash_t(value) *self, char *key);

int
ifj17_hash_has(khash_t(value) *self, char *key);

void
ifj17_hash_remove(khash_t(value) *self, char *key);

#endif /* IFJ17_HASH_H */
