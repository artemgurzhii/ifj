//
// object.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_OBJECT_H
#define IFJ17_OBJECT_H

#include "hash.h"
#include <stdbool.h>

/*
 * Check if `val` is the given type.
 */

#define ifj17_object_is(val, t) ((val)->type == IFJ17_TYPE_##t)

/*
 * Specific type macros.
 */

#define ifj17_is_node(val) ifj17_object_is(val, NODE)
#define ifj17_is_array(val) ifj17_object_is(val, ARRAY)
#define ifj17_is_object(val) ifj17_object_is(val, OBJECT)
#define ifj17_is_string(val) ifj17_object_is(val, STRING)
#define ifj17_is_double(val) ifj17_object_is(val, DOUBLE)
#define ifj17_is_int(val) ifj17_object_is(val, INT)
#define ifj17_is_bool(val) ifj17_object_is(val, BOOL)
#define ifj17_is_null(val) ifj17_object_is(val, NULL)

/*
 * IFJ17 value types.
 */

typedef enum
{
  IFJ17_TYPE_NULL,
  IFJ17_TYPE_NODE,
  IFJ17_TYPE_BOOL,
  IFJ17_TYPE_INT,
  IFJ17_TYPE_DOUBLE,
  IFJ17_TYPE_STRING,
  IFJ17_TYPE_OBJECT,
  IFJ17_TYPE_ARRAY, // delete
  IFJ17_TYPE_LIST // delete
} ifj17_object;

/*
 * IFJ17 object.
 *
 * A simple tagged union forming
 * the basis of a IFJ17 values.
 */

struct ifj17_object_struct
{
  ifj17_object type;
  union {
    void *as_pointer;
    int as_int;
    double as_double;
  } value;
};

// protos

void ifj17_object_inspect(ifj17_object_t *self);

ifj17_object_t *ifj17_int_new(int val);

ifj17_object_t *ifj17_double_new(double val);

ifj17_object_t *ifj17_bool_new(bool val);

ifj17_object_t *ifj17_string_new(const char *val);

void ifj17_object_free(ifj17_object_t *self);

#endif /* IFJ17_OBJECT_H */
