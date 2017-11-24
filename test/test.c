
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

/*
 * Unit test the given `fn`.
 */

#define unit_test(fn)                                                               \
  printf("    \e[92m✓ \e[90m%s\e[0m\n", #fn);                                       \
  unit_test_##fn();

/*
 * Integration test the given `fn`.
 */

#define integration_test(fn)                                                        \
  printf("    \e[92m✓ \e[90m%s\e[0m\n", #fn);                                       \
  integration_test_##fn();

/*
 * Test suite title.
 */

#define suite(title) printf("\n  \e[36m%s\e[0m\n", title)

/*
 * Test type.
 */

#define type(title) printf("\n  \e[1;33m\033[1m%s\033[0m\e[0m\n", title)

/*
 * Report sizeof.
 */

#define size(type) printf("\n  \e[90m%s: %ld bytes\e[0m\n", #type, sizeof(type));

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

static void unit_test_value_is() {
  ifj17_object_t one = {.type = IFJ17_TYPE_INT, .value.as_int = 1};
  assert(ifj17_is_int(&one));
  assert(!ifj17_is_string(&one));

  ifj17_object_t two = {.type = IFJ17_TYPE_NULL};
  assert(ifj17_is_null(&two));
}

/*
 * Test ifj17_vec_length().
 */

static void unit_test_array_length() {
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

static void unit_test_array_push() {
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

static void unit_test_array_at() {
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

static void unit_test_array_iteration() {
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

static void unit_test_hash_set() {
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

static void unit_test_hash_has() {
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

static void unit_test_hash_remove() {
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

static void unit_test_hash_iteration() {
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

static void unit_test_hash_mixins() {
  ifj17_object_t type = {.type = IFJ17_TYPE_INT, .value.as_int = 1};
  ifj17_vec_t arr;
  ifj17_vec_init(&arr);
}

/*
 * Test strings.
 */

static void unit_test_string() {
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

  size_t ln = strlen(print_buf) - 1;
  if (*print_buf && print_buf[ln] == '\n') {
    strcat(expected, "\n");
  }

  assert(strcmp(expected, print_buf) == 0);
}

// NOTE: UNIT TESTS
static void unit_test_variable_declaration() {
  _test_parser("test/parser/variables/declaration.ifj17",
               "test/parser/variables/declaration.out");
}

static void unit_test_variable_declaration_and_assignment() {
  _test_parser("test/parser/variables/declaration-and-assignment.ifj17",
               "test/parser/variables/declaration-and-assignment.out");
}

static void unit_test_function_declaration_without_arguments() {
  _test_parser("test/parser/function/declaration/without-arguments.ifj17",
               "test/parser/function/declaration/without-arguments.out");
}

static void unit_test_function_declaration_with_argument() {
  _test_parser("test/parser/function/declaration/with-argument.ifj17",
               "test/parser/function/declaration/with-argument.out");
}

static void unit_test_function_declaration_with_arguments() {
  _test_parser("test/parser/function/declaration/with-arguments.ifj17",
               "test/parser/function/declaration/with-arguments.out");
}

static void unit_test_function_declaration_with_body() {
  _test_parser("test/parser/function/declaration/with-body.ifj17",
               "test/parser/function/declaration/with-body.out");
}

// NOTE: INTEGRATION TESTS
static void integration_test_function_initialization() {
  _test_parser("test/integration/parser/function-initialization.ifj17",
               "test/integration/parser/function-initialization.out");
}

// static void unit_test_assign() {
//   _test_parser("test/parser/assign.ifj17", "test/parser/assign.out");
// }
//
// static void unit_test_assign_chain() {
//   _test_parser("test/parser/assign.chain.ifj17", "test/parser/assign.chain.out");
// }
//
// static void unit_test_subscript() {
//   _test_parser("test/parser/subscript.ifj17", "test/parser/subscript.out");
// }
//
// static void unit_test_return() {
//   _test_parser("test/parser/return.ifj17", "test/parser/return.out");
// }
// static void test_err_expr(){
//   _test_parser("test/parser/err_parser/expr.ifj", "test/parser/err_parser/expr.out");
// }
// static void test_err_paren_expr(){
//   _test_parser("test/parser/err_parser/paren_expr.ifj", "test/parser/err_parser/paren_expr.out");
// }
// static void test_err_as(){
//   _test_parser("test/parser/err_parser/decl_expr/as.ifj", "test/parser/err_parser/decl_expr/as.out");
// }
// static void test_err_dim(){
//   _test_parser("test/parser/err_parser/decl_expr/dim.ifj", "test/parser/err_parser/decl_expr/dim.out");
// }
// static void test_err_var_id(){
//   _test_parser("test/parser/err_parser/decl_expr/var_id.ifj", "test/parser/err_parser/decl_expr/var_id.out");
// }
// static void test_err_var_type(){
//   _test_parser("test/parser/err_parser/decl_expr/var_type.ifj", "test/parser/err_parser/decl_expr/var_type.out");
// }
// static void test_err_func_id(){
//   _test_parser("test/parser/err_parser/function_stmt/func_id.ifj", "test/parser/err_parser/function_stmt/func_id.out");
// }
// static void test_err_rparen(){
//   _test_parser("test/parser/err_parser/function_stmt/rparen.ifj", "test/parser/err_parser/function_stmt/rparen.out");
// }
// static void test_err_chr(){
//    _test_parser("test/lexer/error_chr.ifj", "test/lexer/error_chr.out");
// }
//
// static void test_err_hex(){
//    _test_parser("test/lexer/hex/error_hex.ifj", "test/lexer/hex/error_hex.out");
// }
//
// static void test_err_hex1(){
//    _test_parser("test/lexer/hex/error_hex1.ifj", "test/lexer/hex/error_hex1.out");
// }

/*
 * Run all test suites.
 */

int main(int argc, const char **argv) {
  clock_t start = clock();

  size(ifj17_object_t);

  type("UNIT TESTS");

  suite("value");
  unit_test(value_is);

  suite("array");
  unit_test(array_length);
  unit_test(array_push);
  unit_test(array_at);
  unit_test(array_iteration);

  suite("hash");
  unit_test(hash_set);
  unit_test(hash_has);
  unit_test(hash_remove);
  unit_test(hash_iteration);
  unit_test(hash_mixins);

  suite("string");

  unit_test(string);

  suite("parser");
  
  test(string);

  // test(assign);
  // test(assign_chain);
  // test(subscript);
  // test(declaration);
  // test(return );
  
  // test(err_chr);
  // test(err_hex);
  // test(err_hex1);
  
  // test(err_expr);
  // test(err_paren_expr);
  // test(err_as);
  // test(err_dim);
  // test(err_var_type);
  // test(err_func_id);
  // test(err_var_id);
  // test(err_rparen);
  
  // NOTE: Variable tests
  unit_test(variable_declaration);
  unit_test(variable_declaration_and_assignment);

  // NOTE: Function tests
  unit_test(function_declaration_without_arguments);
  unit_test(function_declaration_with_argument);
  unit_test(function_declaration_with_arguments);
  unit_test(function_declaration_with_body);

  // unit_test(assign);
  // unit_test(assign_chain);
  // unit_test(subscript);
  // unit_test(return );

  type("INTEGRATION TESTS");

  suite("parser");
  integration_test(function_initialization);

  printf("\n");
  printf("  \e[90mcompleted in \e[32m%.5fs\e[0m\n",
         (double)(clock() - start) / CLOCKS_PER_SEC);
  printf("\n");
  return 0;
}
