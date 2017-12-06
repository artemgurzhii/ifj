#include "codegen.h"
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
#include "vm.h"
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

#define acceptance_test(fn)                                                         \
  printf("    \e[92m✓ \e[90m%s\e[0m\n", #fn);                                       \
  acceptance_test_##fn();

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

static void _test_parser(const char *base_path) {
  ifj17_lexer_t lexer;
  ifj17_parser_t parser;
  ifj17_block_node_t *root;

  char source_path[100] = "";
  char out_path[100] = "";

  strcat(source_path, base_path);
  strcat(out_path, base_path);

  strcat(source_path, ".ifj17");
  strcat(out_path, ".out");

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

  // DEBUG
  // printf("%s\n", print_buf);
  // printf("%s\n", expected);

  size_t ln = strlen(print_buf) - 1;
  if (*print_buf && print_buf[ln] == '\n') {
    strcat(expected, "\n");
  }

  assert(strcmp(expected, print_buf) == 0);
}

// Test code generator

static void _test_codegen(const char *base_path) {
  ifj17_lexer_t lexer;
  ifj17_parser_t parser;
  ifj17_block_node_t *root;

  char source_path[100] = "";
  char out_path[100] = "";

  strcat(source_path, base_path);
  strcat(out_path, base_path);

  strcat(source_path, ".ifj17");
  strcat(out_path, ".out");

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
  ifj17_vm_t *vm = ifj17_gen((ifj17_node_t *)root);

  // ifj17_object_t *obj = ifj17_eval(vm);
  //
  // ifj17_object_inspect(obj);
  // ifj17_object_free(obj);
  ifj17_vm_free(vm);

  // DEBUG
  // printf("%s\n", print_buf);
  // printf("%s\n", expected);

  size_t ln = strlen(print_buf) - 1;
  if (*print_buf && print_buf[ln] == '\n' && print_buf[ln - 1] == '\n') {
    strcat(expected, "\n");
  }

  assert(strcmp(expected, print_buf) == 0);
}

// NOTE: UNIT TESTS

// PARSER

// VARIABLES
static void unit_test_variable_declaration() {
  _test_parser("test/unit/parser/variables/declaration");
}

static void unit_test_variable_declaration_and_assignment() {
  _test_parser("test/unit/parser/variables/declaration-and-assignment");
}

static void unit_test_variable_assign() {
  _test_parser("test/unit/parser/variables/assign");
}

static void unit_test_variable_assign_chain() {
  _test_parser("test/unit/parser/variables/assign-chain");
}

// FUNCTIONS
static void unit_test_function_declaration_without_arguments() {
  _test_parser("test/unit/parser/function/declaration/without-arguments");
}

static void unit_test_function_declaration_with_argument() {
  _test_parser("test/unit/parser/function/declaration/with-argument");
}

static void unit_test_function_declaration_with_arguments() {
  _test_parser("test/unit/parser/function/declaration/with-arguments");
}

static void unit_test_function_initialization_without_arguments() {
  _test_parser("test/unit/parser/function/initialization/without-arguments");
}

static void unit_test_function_initialization_with_argument() {
  _test_parser("test/unit/parser/function/initialization/with-argument");
}

static void unit_test_function_initialization_with_arguments() {
  _test_parser("test/unit/parser/function/initialization/with-arguments");
}

static void unit_test_function_initialization_with_body() {
  _test_parser("test/unit/parser/function/initialization/with-body");
}

// SCOPE
static void unit_test_scope_empty_declaration() {
  _test_parser("test/unit/parser/scope/declaration/empty");
}

static void unit_test_scope_with_body() {
  _test_parser("test/unit/parser/scope/declaration/with-body");
}

// COMMENTS
static void unit_test_comments_inline() {
  _test_parser("test/unit/parser/comments/inline");
}

static void unit_test_comments_multiline_with_code() {
  _test_parser("test/unit/parser/comments/multiline-with-code");
}

static void unit_test_comments_multiline_only_comments() {
  _test_parser("test/unit/parser/comments/multiline-only-comments");
}

static void unit_test_comments_only_comments() {
  _test_parser("test/unit/parser/comments/only-comments");
}

static void unit_test_comments_without_spaces() {
  _test_parser("test/unit/parser/comments/without-spaces");
}

static void unit_test_comments_block() {
  _test_parser("test/unit/parser/comments/block");
}

// OPERATIONS
static void unit_test_operation_plus() {
  _test_parser("test/unit/parser/operations/plus");
}

// CONDITIONS
static void unit_test_if_single() {
  _test_parser("test/unit/parser/conditions/if-singles");
}

static void unit_test_if_else() {
  _test_parser("test/unit/parser/conditions/if-else");
}

static void unit_test_if_elseif() {
  _test_parser("test/unit/parser/conditions/if-elseif");
}

static void unit_test_if_elseif_else() {
  _test_parser("test/unit/parser/conditions/if-elseif-else");
}

// STRINGS
static void unit_test_empty_string() {
  _test_parser("test/unit/parser/string/empty-string");
}

static void unit_test_escape_new_line() {
  _test_parser("test/unit/parser/string/escape-new-line");
}

static void unit_test_escape_quote() {
  _test_parser("test/unit/parser/string/escape-quote");
}

static void unit_test_escape_sequence() {
  _test_parser("test/unit/parser/string/escape-sequence");
}

static void unit_test_simple_string() {
  _test_parser("test/unit/parser/string/simple-string");
}

static void unit_test_long_string() {
  _test_parser("test/unit/parser/string/long-string");
}

static void unit_test_escape_line_break() {
  _test_parser("test/unit/parser/string/escape-line-break");
}

// CASE insensitive

static void unit_test_case_insensitive_variable_declaration() {
  _test_parser("test/unit/parser/case-insensitive/variable-declaration");
}

static void unit_test_case_insensitive_if_elseif_else() {
  _test_parser("test/unit/parser/case-insensitive/if-elseif-else");
}

static void unit_test_case_insensitive_function_initialization_with_body() {
  _test_parser("test/unit/parser/case-insensitive/function-with-body");
}

static void unit_test_case_insensitive_scope_with_body() {
  _test_parser("test/unit/parser/case-insensitive/scope-with-body");
}

static void unit_test_case_insensitive_string() {
  _test_parser("test/unit/parser/case-insensitive/string");
}

// LOOPS
static void unit_test_do_while_empty() {
  _test_parser("test/unit/parser/loops/empty");
}

static void unit_test_do_while_with_body() {
  _test_parser("test/unit/parser/loops/with-body");
}

// CODEGEN
// OPERATIONS
static void acceptance_test_arithmetic_operators() {
  _test_codegen("test/acceptance/binary_operators/arithmetic");
}

static void acceptance_test_boolean_operators() {
  _test_codegen("test/acceptance/binary_operators/boolean");
}

static void acceptance_test_relation_operators() {
  _test_codegen("test/acceptance/binary_operators/relations");
}

static void acceptance_test_unary_minus() {
  _test_codegen("test/acceptance/unary_operators/minus");
}

static void acceptance_test_division() {
  _test_codegen("test/acceptance/binary_operators/division");
}

// DECLARATION OF VARIABLES

static void acceptance_test_assignment_vars() {
  _test_codegen("test/acceptance/assignment_vars/assignment");
}

// TYPES CONTROL
static void acceptance_test_types_control_arithmetic() {
  _test_codegen("test/acceptance/types_control/add_sub_mul");
}

static void acceptance_test_types_control_relation() {
  _test_codegen("test/acceptance/types_control/relation_op");
}

static void acceptance_test_types_control_jump_if() {
  _test_codegen("test/acceptance/types_control/jump_if");
}

// LOOPS

static void acceptance_test_do_while_whithout_body() {
  _test_codegen("test/acceptance/loops/while/without-body");
}

static void acceptance_test_do_while_with_body() {
  _test_codegen("test/acceptance/loops/while/with-body");
}

// BUILT-IN METHODS
static void unit_test_built_in_print_multiple_of_int() {
  _test_parser("test/unit/parser/built-in/print/multiple-of-int");
}

static void unit_test_built_in_print_multiple_of_string() {
  _test_parser("test/unit/parser/built-in/print/multiple-of-string");
}

static void unit_test_built_in_print_multiple_of_variable() {
  _test_parser("test/unit/parser/built-in/print/multiple-of-variable");
}

static void unit_test_built_in_print_multiple_of_random_types() {
  _test_parser("test/unit/parser/built-in/print/multiple-of-random-types");
}

static void unit_test_built_in_print_single_int() {
  _test_parser("test/unit/parser/built-in/print/single-int");
}

static void unit_test_built_in_print_single_string() {
  _test_parser("test/unit/parser/built-in/print/single-string");
}

static void unit_test_built_in_print_single_variable() {
  _test_parser("test/unit/parser/built-in/print/single-variable");
}

// NOTE: INTEGRATION TESTS
static void integration_test_factorial() {
  _test_parser("test/integration/parser/factorial");
}

static void integration_test_case_insensitive_factorial() {
  _test_parser("test/integration/parser/case-insensitive-factorial");
}

static void acceptance_test_do_while_nested() {
  _test_codegen("test/acceptance/loops/while/nested-loo");
}

static void acceptance_test_do_while_empty2x() {
  _test_codegen("test/acceptance/loops/while/without-body2x");
}

static void acceptance_test_do_while_with_body2x() {
  _test_codegen("test/acceptance/loops/while/with-body2x");
}

static void acceptance_test_do_while_nested2x() {
  _test_codegen("test/acceptance/loops/while/nested-loop2x");
}

// CONDITIONS
static void acceptance_test_if_single() {
  _test_codegen("test/acceptance/conditions/if-single");
}

static void acceptance_test_if_else() {
  _test_codegen("test/acceptance/conditions/if-else");
}

static void acceptance_test_if_elseif() {
  _test_codegen("test/acceptance/conditions/if-elseif");
}

static void acceptance_test_if_elseif_else() {
  _test_codegen("test/acceptance/conditions/if-elseif-else");
}

static void acceptance_test_if_single2x() {
  _test_codegen("test/acceptance/conditions/if-single2x");
}

static void acceptance_test_if_else2x() {
  _test_codegen("test/acceptance/conditions/if-else2x");
}

static void acceptance_test_if_elseif2x() {
  _test_codegen("test/acceptance/conditions/if-elseif2x");
}

static void acceptance_test_if_elseif_else2x() {
  _test_codegen("test/acceptance/conditions/if-elseif-else2x");
}

// FUNCTIONS
static void acceptance_test_function_simple() {
  _test_codegen("test/acceptance/functions/function-simple");
}

static void acceptance_test_function_simple_with_args() {
  _test_codegen("test/acceptance/functions/function-simple-with-args");
}

static void acceptance_test_function_local_vars() {
  _test_codegen("test/acceptance/functions/function-local-vars");
}

static void acceptance_test_factorial() {
  _test_codegen("test/acceptance/functions/factorial");
}

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

  // NOTE:
  // Comment tests unit_test(comments_only_comments);
  unit_test(comments_multiline_only_comments);
  unit_test(comments_inline);
  unit_test(comments_without_spaces);
  unit_test(comments_multiline_with_code);
  unit_test(comments_block);

  // NOTE: Variable tests
  unit_test(variable_declaration);
  unit_test(variable_declaration_and_assignment);
  unit_test(variable_assign);
  unit_test(variable_assign_chain);

  // NOTE: Function tests
  // Declaration
  unit_test(function_declaration_without_arguments);
  unit_test(function_declaration_with_argument);
  unit_test(function_declaration_with_arguments);

  // Initialization
  unit_test(function_initialization_without_arguments);
  unit_test(function_initialization_with_argument);
  unit_test(function_initialization_with_arguments);
  unit_test(function_initialization_with_body);

  // NOTE: Scope tests
  unit_test(scope_empty_declaration);
  unit_test(scope_with_body);

  // NOTE: Conditions test
  unit_test(if_single);
  unit_test(if_else);
  unit_test(if_elseif);
  unit_test(if_elseif_else);

  // NOTE: Loop tests
  unit_test(do_while_empty);
  unit_test(do_while_with_body);

  // NOTE: String test
  unit_test(empty_string);
  unit_test(escape_new_line);
  unit_test(escape_quote);
  unit_test(escape_sequence);
  // unit_test(long_string);
  unit_test(simple_string);
  unit_test(escape_line_break);

  // NOTE: Case insensitive tests
  unit_test(case_insensitive_variable_declaration);
  unit_test(case_insensitive_if_elseif_else);
  unit_test(case_insensitive_function_initialization_with_body);
  unit_test(case_insensitive_scope_with_body);
  unit_test(case_insensitive_string);

  // NOTE: Build in tests
  unit_test(built_in_print_single_int);
  unit_test(built_in_print_single_string);
  unit_test(built_in_print_single_variable);
  unit_test(built_in_print_multiple_of_int);
  unit_test(built_in_print_multiple_of_string);
  unit_test(built_in_print_multiple_of_variable);
  unit_test(built_in_print_multiple_of_random_types);

  type("INTEGRATION TESTS");

  suite("parser");
  integration_test(factorial);
  integration_test(case_insensitive_factorial);

  type("ACCEPTANCE TESTS");

  suite("operations");
  acceptance_test(arithmetic_operators);
  acceptance_test(boolean_operators);
  // acceptance_test(unary_minus);
  // acceptance_test(relation_operators);
  acceptance_test(division);

  suite("types_control");
  acceptance_test(types_control_arithmetic);
  // acceptance_test(types_control_relation);
  acceptance_test(types_control_jump_if);

  suite("assignment");
  // acceptance_test(assignment_vars);

  suite("conditions");
  // acceptance_test(if_single);
  // acceptance_test(if_else);
  // acceptance_test(if_elseif);
  // acceptance_test(if_elseif_else);
  // acceptance_test(if_single2x);
  // acceptance_test(if_else2x);
  // acceptance_test(if_elseif2x);
  // acceptance_test(if_elseif_else2x);

  suite("loops");
  // acceptance_test(do_while_whithout_body);
  // acceptance_test(do_while_with_body);
  // acceptance_test(do_while_nested);
  // acceptance_test(do_while_empty2x);
  // acceptance_test(do_while_with_body2x);
  // acceptance_test(do_while_nested2x);

  suite("functions");
  // acceptance_test(function_simple);
  // acceptance_test(function_simple_with_args);
  // acceptance_test(function_local_vars);
  // acceptance_test(factorial);

  printf("\n");
  printf("  \e[90mcompleted in \e[32m%.5fs\e[0m\n",
         (double)(clock() - start) / CLOCKS_PER_SEC);
  printf("\n");
  return 0;
}
