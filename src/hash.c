//
// hash.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "hash.h"

/*
 * Set hash `key` to `val`.
 */

inline void ifj17_hash_set(khash_t(value) * self, char *key, ifj17_object_t *val) {
  int ret;
  khiter_t k = kh_put(value, self, key, &ret);
  kh_value(self, k) = val;
}

/*
 * Get hash `key`, or NULL.
 */

inline ifj17_object_t *ifj17_hash_get(khash_t(value) * self, char *key) {
  khiter_t k = kh_get(value, self, key);
  return k == kh_end(self) ? NULL : kh_value(self, k);
}

/*
 * Check if hash `key` exists.
 */

inline int ifj17_hash_has(khash_t(value) * self, char *key) {
  khiter_t k = kh_get(value, self, key);
  return kh_exist(self, k);
}

/*
 * Remove hash `key`.
 */

void ifj17_hash_remove(khash_t(value) * self, char *key) {
  khiter_t k = kh_get(value, self, key);
  kh_del(value, self, k);
}
