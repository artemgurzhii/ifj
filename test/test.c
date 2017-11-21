
#include "errors.h"
#include "hash.h"
#include "khash.h"
#include "lexer.h"
#include "object.h"
#include "parser.h"
#include "prettyprint.h"
#include "state.h"
#include "utils.h"
#include "vec.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// print func for prettyprint

char *print_buf;

int bprintf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int r = vsprintf(print_buf + strlen(print_buf), format, ap);
  va_end(ap);
  return r;
}

/*
 * Test ifj17_is_* macros.
 */

static void test_value_is() {
  ifj17_object_t one = {.type = IFJ17_TYPE_INT, .value.as_int = 1};
  assert(ifj17_is_int(&one));
  assert(!ifj17_is_string(&one));

  ifj17_object_t two = {.type = IFJ17_TYPE_NULL};
  assert(ifj17_is_null(&two));
}

/*
 * Test ifj17_vec_length().
 */

static void test_array_length() {
  ifj17_vec_t arr;
  ifj17_vec_init(&arr);

  ifj17_object_t one = {.type = IFJ17_TYPE_INT, .value.as_int = 1};
  ifj17_object_t two = {.type = IFJ17_TYPE_INT, .value.as_int = 2};
  ifj17_object_t three = {.type = IFJ17_TYPE_INT, .value.as_int = 3};

  assert(ifj17_vec_length(&arr) == 0);

  ifj17_vec_push(&arr, &one);
  assert(ifj17_vec_length(&arr) == 1);

  ifj17_vec_push(&arr, &two);
  assert(ifj17_vec_length(&arr) == 2);

  ifj17_vec_push(&arr, &three);
  assert(ifj17_vec_length(&arr) == 3);
}

/*
 * Test ifj17_vec_push().
 */

static void test_array_push() {
  ifj17_vec_t arr;
  ifj17_vec_init(&arr);

  ifj17_object_t one = {.type = IFJ17_TYPE_INT, .value.as_int = 1};
  ifj17_object_t two = {.type = IFJ17_TYPE_INT, .value.as_int = 2};
  ifj17_object_t three = {.type = IFJ17_TYPE_INT, .value.as_int = 3};

  assert(ifj17_vec_length(&arr) == 0);

  ifj17_vec_push(&arr, &one);
  assert(ifj17_vec_pop(&arr)->value.as_int == 1);

  ifj17_vec_push(&arr, &one);
  ifj17_vec_push(&arr, &one);
  assert(ifj17_vec_pop(&arr)->value.as_int == 1);
  assert(ifj17_vec_pop(&arr)->value.as_int == 1);

  ifj17_vec_push(&arr, &one);
  ifj17_vec_push(&arr, &two);
  ifj17_vec_push(&arr, &three);
  assert(ifj17_vec_pop(&arr)->value.as_int == 3);
  assert(ifj17_vec_pop(&arr)->value.as_int == 2);
  assert(ifj17_vec_pop(&arr)->value.as_int == 1);

  assert(ifj17_vec_pop(&arr) == NULL);
  assert(ifj17_vec_pop(&arr) == NULL);
  assert(ifj17_vec_pop(&arr) == NULL);
  ifj17_vec_push(&arr, &one);
  assert(ifj17_vec_pop(&arr)->value.as_int == 1);
}

/*
 * Test ifj17_vec_at().
 */

static void test_array_at() {
  ifj17_vec_t arr;
  ifj17_vec_init(&arr);

  ifj17_object_t one = {.type = IFJ17_TYPE_INT, .value.as_int = 1};
  ifj17_object_t two = {.type = IFJ17_TYPE_INT, .value.as_int = 2};
  ifj17_object_t three = {.type = IFJ17_TYPE_INT, .value.as_int = 3};

  ifj17_vec_push(&arr, &one);
  ifj17_vec_push(&arr, &two);
  ifj17_vec_push(&arr, &three);

  assert(ifj17_vec_at(&arr, 0)->value.as_int == 1);
  assert(ifj17_vec_at(&arr, 1)->value.as_int == 2);
  assert(ifj17_vec_at(&arr, 2)->value.as_int == 3);

  assert(ifj17_vec_at(&arr, -1123) == NULL);
  assert(ifj17_vec_at(&arr, 5) == NULL);
  assert(ifj17_vec_at(&arr, 1231231) == NULL);
}

/*
 * Test array iteration.
 */

static void test_array_iteration() {
  ifj17_vec_t arr;
  ifj17_vec_init(&arr);

  ifj17_object_t one = {.type = IFJ17_TYPE_INT, .value.as_int = 1};
  ifj17_object_t two = {.type = IFJ17_TYPE_INT, .value.as_int = 2};
  ifj17_object_t three = {.type = IFJ17_TYPE_INT, .value.as_int = 3};

  ifj17_vec_push(&arr, &one);
  ifj17_vec_push(&arr, &two);
  ifj17_vec_push(&arr, &three);

  int vals[3];
  int k = 0;

  ifj17_vec_each(&arr, { vals[k++] = val->value.as_int; });
  assert(vals[0] == 1);
  assert(vals[1] == 2);
  assert(vals[2] == 3);
}

/*
 * Test ifj17_hash_set().
 */

static void test_hash_set() {
  ifj17_object_t one = {.type = IFJ17_TYPE_INT, .value.as_int = 1};
  ifj17_object_t two = {.type = IFJ17_TYPE_INT, .value.as_int = 2};
  ifj17_object_t three = {.type = IFJ17_TYPE_INT, .value.as_int = 3};

  ifj17_hash_t *obj = ifj17_hash_new();

  assert(ifj17_hash_size(obj) == 0);

  ifj17_hash_set(obj, "one", &one);
  assert(ifj17_hash_size(obj) == 1);

  ifj17_hash_set(obj, "two", &two);
  assert(ifj17_hash_size(obj) == 2);

  ifj17_hash_set(obj, "three", &three);
  assert(ifj17_hash_size(obj) == 3);

  assert(ifj17_hash_get(obj, "one") == &one);
  assert(ifj17_hash_get(obj, "two") == &two);
  assert(ifj17_hash_get(obj, "three") == &three);
  assert(ifj17_hash_get(obj, "four") == NULL);

  ifj17_hash_destroy(obj);
}

/*
 * Test ifj17_hash_has().
 */

static void test_hash_has() {
  ifj17_object_t one = {.type = IFJ17_TYPE_INT, .value.as_int = 1};

  ifj17_hash_t *obj = ifj17_hash_new();

  ifj17_hash_set(obj, "one", &one);

  assert(1 == ifj17_hash_has(obj, "one"));
  assert(0 == ifj17_hash_has(obj, "foo"));

  ifj17_hash_destroy(obj);
}

/*
 * Test ifj17_hash_remove().
 */

static void test_hash_remove() {
  ifj17_object_t one = {.type = IFJ17_TYPE_INT, .value.as_int = 1};

  ifj17_hash_t *obj = ifj17_hash_new();

  ifj17_hash_set(obj, "one", &one);
  assert(&one == ifj17_hash_get(obj, "one"));

  ifj17_hash_remove(obj, "one");
  assert(NULL == ifj17_hash_get(obj, "one"));

  ifj17_hash_set(obj, "one", &one);
  assert(&one == ifj17_hash_get(obj, "one"));

  ifj17_hash_remove(obj, "one");
  assert(NULL == ifj17_hash_get(obj, "one"));

  ifj17_hash_destroy(obj);
}

/*
 * Check if the given `slot` is valid.
 */

static int valid_slot(const char *slot) {
  return strcmp("one", slot) == 0 || strcmp("two", slot) == 0 ||
         strcmp("three", slot) == 0 || strcmp("four", slot) == 0 ||
         strcmp("five", slot) == 0;
}

/*
 * Test object iteration macros.
 */

static void test_hash_iteration() {
  ifj17_object_t one = {.type = IFJ17_TYPE_INT, .value.as_int = 1};
  ifj17_object_t two = {.type = IFJ17_TYPE_INT, .value.as_int = 2};
  ifj17_object_t three = {.type = IFJ17_TYPE_INT, .value.as_int = 3};

  ifj17_hash_t *obj = ifj17_hash_new();

  assert(0 == ifj17_hash_size(obj));

  ifj17_hash_set(obj, "one", &one);
  ifj17_hash_set(obj, "two", &two);
  ifj17_hash_set(obj, "three", &three);
  ifj17_hash_set(obj, "four", &three);
  ifj17_hash_set(obj, "five", &three);

  const char *slots[ifj17_hash_size(obj)];
  int i = 0;
  ifj17_hash_each_slot(obj, { slots[i++] = slot; });
  for (int i = 0; i < 5; ++i)
    assert(valid_slot(slots[i]));

  const char *slots2[ifj17_hash_size(obj)];
  i = 0;
  ifj17_hash_each_slot(obj, slots2[i++] = slot);
  for (int i = 0; i < 5; ++i)
    assert(valid_slot(slots2[i]));

  const char *slots3[ifj17_hash_size(obj)];
  i = 0;
  ifj17_hash_each(obj, slots3[i++] = slot);
  for (int i = 0; i < 5; ++i)
    assert(valid_slot(slots3[i]));

  ifj17_hash_destroy(obj);
}

/*
 * Test mixins.
 */

static void test_hash_mixins() {
  ifj17_object_t type = {.type = IFJ17_TYPE_INT, .value.as_int = 1};
  ifj17_vec_t arr;
  ifj17_vec_init(&arr);
}

/*
 * Test strings.
 */

static void test_string() {
  ifj17_state_t state;
  ifj17_state_init(&state);

  ifj17_string_t *str = ifj17_string(&state, "foo bar baz");
  assert(strcmp("foo bar baz", str->val) == 0);

  str = ifj17_string(&state, "foo bar baz");
  assert(strcmp("foo bar baz", str->val) == 0);

  for (int i = 0; i < 200; ++i)
    str = ifj17_string(&state, "foo");
  assert(strcmp("foo", str->val) == 0);

  assert(kh_size(state.strs) == 2);
}

/*
 * Test parser.
 */

static void _test_parser(const char *source_path, const char *out_path) {
  ifj17_lexer_t lexer;
  ifj17_parser_t parser;
  ifj17_block_node_t *root;

  char *source = file_read(source_path);
  assert(source != NULL);
  char *expected = file_read(out_path);
  assert(expected != NULL);

  ifj17_lexer_init(&lexer, source, source_path);
  ifj17_parser_init(&parser, &lexer);

  if (!(root = ifj17_parse(&parser))) {
    ifj17_report_error(&parser);
    exit(1);
  }

  char buf[1024] = {0};
  print_buf = buf;
  ifj17_set_prettyprint_func(bprintf);
  ifj17_prettyprint((ifj17_node_t *)root);

  assert(strcmp(expected, print_buf) == 0);
}

static void test_assign() {
  _test_parser("test/parser/assign.ifj17", "test/parser/assign.out");
}

static void test_assign_chain() {
  _test_parser("test/parser/assign.chain.ifj17", "test/parser/assign.chain.out");
}

static void test_subscript() {
  _test_parser("test/parser/subscript.ifj17", "test/parser/subscript.out");
}

static void test_declaration() {
  _test_parser("test/parser/declaration.ifj17", "test/parser/declaration.out");
}

static void test_return() {
  _test_parser("test/parser/return.ifj17", "test/parser/return.out");
}

static void test_use() {
  _test_parser("test/parser/use.ifj17", "test/parser/use.out");
}

/*
 * Test the given `fn`.
 */

#define test(fn)                                                                    \
  printf("    \e[92m✓ \e[90m%s\e[0m\n", #fn);                                       \
  test_##fn();

/*
 * Test suite title.
 */

#define suite(title) printf("\n  \e[36m%s\e[0m\n", title)

/*
 * Report sizeof.
 */

#define size(type) printf("\n  \e[90m%s: %ld bytes\e[0m\n", #type, sizeof(type));

/*
 * Run all test suites.
 */

int main(int argc, const char **argv) {
  clock_t start = clock();

  size(ifj17_object_t);

  suite("value");
  test(value_is);

  suite("array");
  test(array_length);
  test(array_push);
  test(array_at);
  test(array_iteration);

  suite("hash");
  test(hash_set);
  test(hash_has);
  test(hash_remove);
  test(hash_iteration);
  test(hash_mixins);

  suite("string");
  test(string);

  suite("parser");
  test(assign);
  test(assign_chain);
  test(subscript);
  test(declaration);
  test(return );
  test(use);

  printf("\n");
  printf("  \e[90mcompleted in \e[32m%.5fs\e[0m\n",
         (float)(clock() - start) / CLOCKS_PER_SEC);
  printf("\n");
  return 0;
}
