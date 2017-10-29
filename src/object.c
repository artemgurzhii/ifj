//
// object.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "object.h"
#include "internal.h"
#include <assert.h>
#include <stdio.h>

/*
 * Print `self` to stdout.
 */

void ifj17_object_inspect(ifj17_object_t *self) {
  switch (self->type) {
  case IFJ17_TYPE_FLOAT:
    printf("%2f\n", self->value.as_float);
    break;
  case IFJ17_TYPE_INT:
    printf("%d\n", self->value.as_int);
    break;
  case IFJ17_TYPE_BOOL:
    printf("%s\n", self->value.as_int ? "true" : "false");
    break;
  case IFJ17_TYPE_STRING:
    printf("%s\n", (char *)self->value.as_pointer);
    break;
  default:
    assert(0 && "unhandled");
  }
}

/*
 * Allocate an initialize a new object of the given `type`.
 */

static ifj17_object_t *alloc_object(ifj17_object type) {
  ifj17_object_t *self = malloc(sizeof(ifj17_object_t));
  if (unlikely(!self))
    return NULL;
  self->type = type;
  return self;
}

/*
 * Allocate a new int object with the given `val`.
 */

ifj17_object_t *ifj17_int_new(int val) {
  ifj17_object_t *self = alloc_object(IFJ17_TYPE_INT);
  if (unlikely(!self))
    return NULL;
  self->value.as_int = val;
  return self;
}

/*
 * Allocate a new float object with the given `val`.
 */

ifj17_object_t *ifj17_float_new(float val) {
  ifj17_object_t *self = alloc_object(IFJ17_TYPE_FLOAT);
  if (unlikely(!self))
    return NULL;
  self->value.as_float = val;
  return self;
}

/*
 * Allocate a new bool object with the given `val`.
 */

ifj17_object_t *ifj17_bool_new(bool val) {
  ifj17_object_t *self = alloc_object(IFJ17_TYPE_BOOL);
  if (unlikely(!self))
    return NULL;
  self->value.as_int = val;
  return self;
}

/*
 * Allocate a new string object with the given `val`.
 */

ifj17_object_t *ifj17_string_new(const char *val) {
  ifj17_object_t *self = alloc_object(IFJ17_TYPE_STRING);
  if (unlikely(!self))
    return NULL;
  self->value.as_pointer = strdup(val);
  return self;
}

void ifj17_object_free(ifj17_object_t *self) {
  switch (self->type) {
  case IFJ17_TYPE_STRING:
    free(self->value.as_pointer);
    break;
  default:
    break;
  }
  free(self);
}
