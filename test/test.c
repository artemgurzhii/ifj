
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

static void _test_codegen(const char *source_path, const char *out_path) {
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

  char buf[1048] = {0};
  print_buf = buf;
  ifj17_vm_t *vm = ifj17_gen((ifj17_node_t *)root);

  // ifj17_object_t *obj = ifj17_eval(vm);
  //
  // ifj17_object_inspect(obj);
  // ifj17_object_free(obj);
  ifj17_vm_free(vm);

  // DEBUG
  printf("%s\n", print_buf);
  printf("%s\n", expected);

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
  _test_parser("test/unit/parser/variables/declaration.ifj17",
               "test/unit/parser/variables/declaration.out");
}

static void unit_test_variable_declaration_and_assignment() {
  _test_parser("test/unit/parser/variables/declaration-and-assignment.ifj17",
               "test/unit/parser/variables/declaration-and-assignment.out");
}

static void unit_test_variable_assign() {
  _test_parser("test/unit/parser/variables/assign.ifj17",
               "test/unit/parser/variables/assign.out");
}

static void unit_test_variable_assign_chain() {
  _test_parser("test/unit/parser/variables/assign-chain.ifj17",
               "test/unit/parser/variables/assign-chain.out");
}

// FUNCTIONS
static void unit_test_function_declaration_without_arguments() {
  _test_parser("test/unit/parser/function/declaration/without-arguments.ifj17",
               "test/unit/parser/function/declaration/without-arguments.out");
}

static void unit_test_function_declaration_with_argument() {
  _test_parser("test/unit/parser/function/declaration/with-argument.ifj17",
               "test/unit/parser/function/declaration/with-argument.out");
}

static void unit_test_function_declaration_with_arguments() {
  _test_parser("test/unit/parser/function/declaration/with-arguments.ifj17",
               "test/unit/parser/function/declaration/with-arguments.out");
}

static void unit_test_function_initialization_without_arguments() {
  _test_parser("test/unit/parser/function/initialization/without-arguments.ifj17",
               "test/unit/parser/function/initialization/without-arguments.out");
}

static void unit_test_function_initialization_with_argument() {
  _test_parser("test/unit/parser/function/initialization/with-argument.ifj17",
               "test/unit/parser/function/initialization/with-argument.out");
}

static void unit_test_function_initialization_with_arguments() {
  _test_parser("test/unit/parser/function/initialization/with-arguments.ifj17",
               "test/unit/parser/function/initialization/with-arguments.out");
}

static void unit_test_function_initialization_with_body() {
  _test_parser("test/unit/parser/function/initialization/with-body.ifj17",
               "test/unit/parser/function/initialization/with-body.out");
}

// SCOPE
static void unit_test_scope_empty_declaration() {
  _test_parser("test/unit/parser/scope/declaration/empty.ifj17",
               "test/unit/parser/scope/declaration/empty.out");
}

static void unit_test_scope_with_body() {
  _test_parser("test/unit/parser/scope/declaration/with-body.ifj17",
               "test/unit/parser/scope/declaration/with-body.out");
}

// COMMENTS
static void unit_test_comments_inline() {
  _test_parser("test/unit/parser/comments/inline.ifj17",
               "test/unit/parser/comments/inline.out");
}

static void unit_test_comments_multiline_with_code() {
  _test_parser("test/unit/parser/comments/multiline-with-code.ifj17",
               "test/unit/parser/comments/multiline-with-code.out");
}

static void unit_test_comments_multiline_only_comments() {
  _test_parser("test/unit/parser/comments/multiline-only-comments.ifj17",
               "test/unit/parser/comments/multiline-only-comments.out");
}

static void unit_test_comments_only_comments() {
  _test_parser("test/unit/parser/comments/only-comments.ifj17",
               "test/unit/parser/comments/only-comments.out");
}

static void unit_test_comments_without_spaces() {
  _test_parser("test/unit/parser/comments/without-spaces.ifj17",
               "test/unit/parser/comments/without-spaces.out");
}

// CONDITIONS
static void unit_test_if_single() {
  _test_parser("test/unit/parser/conditions/if-single.ifj17",
               "test/unit/parser/conditions/if-single.out");
}

static void unit_test_if_else() {
  _test_parser("test/unit/parser/conditions/if-else.ifj17",
               "test/unit/parser/conditions/if-else.out");
}

static void unit_test_if_elseif() {
  _test_parser("test/unit/parser/conditions/if-elseif.ifj17",
               "test/unit/parser/conditions/if-elseif.out");
}

static void unit_test_if_elseif_else() {
  _test_parser("test/unit/parser/conditions/if-elseif-else.ifj17",
               "test/unit/parser/conditions/if-elseif-else.out");
}

// STRINGS
static void unit_test_empty_string() {
  _test_parser("test/unit/parser/string/empty-string.ifj17",
               "test/unit/parser/string/empty-string.out");
}

static void unit_test_escape_new_line() {
  _test_parser("test/unit/parser/string/escape-new-line.ifj17",
               "test/unit/parser/string/escape-new-line.out");
}

static void unit_test_escape_quote() {
  _test_parser("test/unit/parser/string/escape-quote.ifj17",
               "test/unit/parser/string/escape-quote.out");
}

static void unit_test_escape_sequence() {
  _test_parser("test/unit/parser/string/escape-sequence.ifj17",
               "test/unit/parser/string/escape-sequence.out");
}

static void unit_test_simple_string() {
  _test_parser("test/unit/parser/string/simple-string.ifj17",
               "test/unit/parser/string/simple-string.out");
}

static void unit_test_long_string() {
  _test_parser("test/unit/parser/string/long-string.ifj17",
               "test/unit/parser/string/long-string.out");
}

static void unit_test_escape_line_break() {
  _test_parser("test/unit/parser/string/escape-line-break.ifj17",
               "test/unit/parser/string/escape-line-break.out");
}

// CASE insensitive

static void unit_test_case_insensitive_variable_declaration() {
  _test_parser("test/unit/parser/case-insensitive/variable-declaration.ifj17",
               "test/unit/parser/case-insensitive/variable-declaration.out");
}

static void unit_test_case_insensitive_if_elseif_else() {
  _test_parser("test/unit/parser/case-insensitive/if-elseif-else.ifj17",
               "test/unit/parser/case-insensitive/if-elseif-else.out");
}

static void unit_test_case_insensitive_function_initialization_with_body() {
  _test_parser("test/unit/parser/case-insensitive/function-with-body.ifj17",
               "test/unit/parser/case-insensitive/function-with-body.out");
}

static void unit_test_case_insensitive_scope_with_body() {
  _test_parser("test/unit/parser/case-insensitive/scope-with-body.ifj17",
               "test/unit/parser/case-insensitive/scope-with-body.out");
}

static void unit_test_case_insensitive_string() {
  _test_parser("test/unit/parser/case-insensitive/string.ifj17",
               "test/unit/parser/case-insensitive/string.out");
}

// LOOPS
static void unit_test_do_while_empty() {
  _test_parser("test/unit/parser/loops/empty.ifj17",
               "test/unit/parser/loops/empty.out");
}

static void unit_test_do_while_with_body() {
  _test_parser("test/unit/parser/loops/with-body.ifj17",
               "test/unit/parser/loops/with-body.out");
}

// CODEGEN

// OPERATIONS

static void acceptance_test_arithmetic_operators() {
  _test_codegen("test/acceptance/binary_operators/arithmetic.ifj17",
                "test/acceptance/binary_operators/arithmetic.out");
}

static void acceptance_test_boolean_operators() {
  _test_codegen("test/acceptance/binary_operators/boolean.ifj17",
                "test/acceptance/binary_operators/boolean.out");
}

static void acceptance_test_relation_operators() {
  _test_codegen("test/acceptance/binary_operators/relations.ifj17",
                "test/acceptance/binary_operators/relations.out");
}

static void acceptance_test_unary_minus() {
  _test_codegen("test/acceptance/unary_operators/minus.ifj17",
                "test/acceptance/unary_operators/minus.out");
}

static void acceptance_test_division() {
  _test_codegen("test/acceptance/binary_operators/division.ifj17",
                "test/acceptance/binary_operators/division.out");
}

// DECLARATION OF VARIABLES

static void acceptance_test_assignment_vars() {
  _test_codegen("test/acceptance/decl_vars/assignment.ifj17",
                "test/acceptance/decl_vars/assignment.out");
}

// TYPES CONTROL
static void acceptance_test_types_control_arithmetic() {
  _test_codegen("test/acceptance/types_control/add_sub_mul.ifj17",
                "test/acceptance/types_control/add_sub_mul.out");
}

static void acceptance_test_types_control_relation() {
  _test_codegen("test/acceptance/types_control/relation_op.ifj17",
                "test/acceptance/types_control/relation_op.out");
}

static void acceptance_test_types_control_jump_if() {
  _test_codegen("test/acceptance/types_control/jump_if.ifj17",
                "test/acceptance/types_control/jump_if.out");
}

// LOOPS

static void acceptance_test_do_while_whithout_body() {
  _test_codegen("test/acceptance/loops/while/without-body.ifj17",
                "test/acceptance/loops/while/without-body.out");
}

static void acceptance_test_do_while_with_body() {
  _test_codegen("test/acceptance/loops/while/with-body.ifj17",
                "test/acceptance/loops/while/with-body.out");
}

static void acceptance_test_do_while_nested() {
  _test_codegen("test/acceptance/loops/while/nested-loop.ifj17",
                "test/acceptance/loops/while/nested-loop.out");
}

static void acceptance_test_do_while_empty2x() {
  _test_codegen("test/acceptance/loops/while/without-body2x.ifj17",
                "test/acceptance/loops/while/without-body2x.out");
}

static void acceptance_test_do_while_with_body2x() {
  _test_codegen("test/acceptance/loops/while/with-body2x.ifj17",
                "test/acceptance/loops/while/with-body2x.out");
}

static void acceptance_test_do_while_nested2x() {
  _test_codegen("test/acceptance/loops/while/nested-loop2x.ifj17",
                "test/acceptance/loops/while/nested-loop2x.out");
}

// CONDITIONS
static void acceptance_test_if_single() {
  _test_codegen("test/acceptance/conditions/if-single.ifj17",
                "test/acceptance/conditions/if-single.out");
}

static void acceptance_test_if_else() {
  _test_codegen("test/acceptance/conditions/if-else.ifj17",
                "test/acceptance/conditions/if-else.out");
}

static void acceptance_test_if_elseif() {
  _test_codegen("test/acceptance/conditions/if-elseif.ifj17",
                "test/acceptance/conditions/if-elseif.out");
}

static void acceptance_test_if_elseif_else() {
  _test_codegen("test/acceptance/conditions/if-elseif-else.ifj17",
                "test/acceptance/conditions/if-elseif-else.out");
}

static void acceptance_test_if_single2x() {
  _test_codegen("test/acceptance/conditions/if-single2x.ifj17",
                "test/acceptance/conditions/if-single2x.out");
}

static void acceptance_test_if_else2x() {
  _test_codegen("test/acceptance/conditions/if-else2x.ifj17",
                "test/acceptance/conditions/if-else2x.out");
}

static void acceptance_test_if_elseif2x() {
  _test_codegen("test/acceptance/conditions/if-elseif2x.ifj17",
                "test/acceptance/conditions/if-elseif2x.out");
}

static void acceptance_test_if_elseif_else2x() {
  _test_codegen("test/acceptance/conditions/if-elseif-else2x.ifj17",
                "test/acceptance/conditions/if-elseif-else2x.out");
}

// FUNCTIONS
static void acceptance_test_function_simple() {
  _test_codegen("test/acceptance/functions/function-simple.ifj17",
                "test/acceptance/functions/function-simple.out");
}

static void acceptance_test_function_simple_with_args() {
  _test_codegen("test/acceptance/functions/function-simple-with-args.ifj17",
                "test/acceptance/functions/function-simple-with-args.out");
}

static void acceptance_test_function_local_vars() {
  _test_codegen("test/acceptance/functions/function-local-vars.ifj17",
                "test/acceptance/functions/function-local-vars.out");
}

static void acceptance_test_factorial() {
  _test_codegen("test/acceptance/functions/function-factorial.ifj17",
                "test/acceptance/functions/function-factorial.out");
}

// NOTE: INTEGRATION TESTS
static void integration_test_factorial() {
  _test_parser("test/integration/parser/factorial.ifj17",
               "test/integration/parser/factorial.out");
}

static void integration_test_case_insensitive_factorial() {
  _test_parser("test/integration/parser/case-insensitive-factorial.ifj17",
               "test/integration/parser/case-insensitive-factorial.out");
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

  // NOTE: Comment tests
  // unit_test(comments_only_comments);
  // unit_test(comments_multiline_only_comments);
  // unit_test(comments_inline);
  // unit_test(comments_without_spaces);
  // unit_test(comments_multiline_with_code);
  //
  // // NOTE: Variable tests
  // unit_test(variable_declaration);
  // unit_test(variable_declaration_and_assignment);
  // unit_test(variable_assign);
  // unit_test(variable_assign_chain);
  //
  // // NOTE: Function tests
  // // Declaration
  // unit_test(function_declaration_without_arguments);
  // unit_test(function_declaration_with_argument);
  // unit_test(function_declaration_with_arguments);
  //
  // // Initialization
  // unit_test(function_initialization_without_arguments);
  // unit_test(function_initialization_with_argument);
  // unit_test(function_initialization_with_arguments);
  // unit_test(function_initialization_with_body);
  //
  // // NOTE: Scope tests
  // unit_test(scope_empty_declaration);
  // unit_test(scope_with_body);
  //
  // // NOTE: Conditions test
  // unit_test(if_single);
  // unit_test(if_else);
  // unit_test(if_elseif);
  // unit_test(if_elseif_else);
  //
  // // NOTE: Loop tests
  // unit_test(do_while_empty);
  // unit_test(do_while_with_body);
  //
  // // NOTE: String test
  // unit_test(empty_string);
  // unit_test(escape_new_line);
  // unit_test(escape_quote);
  // unit_test(escape_sequence);
  // // unit_test(long_string);
  // unit_test(simple_string);
  // unit_test(escape_line_break);
  //
  // // NOTE: Case insensitive tests
  // unit_test(case_insensitive_variable_declaration);
  // unit_test(case_insensitive_if_elseif_else);
  // unit_test(case_insensitive_function_initialization_with_body);
  // unit_test(case_insensitive_scope_with_body);
  // unit_test(case_insensitive_string);

  type("INTEGRATION TESTS");

  suite("parser");
  integration_test(factorial);
  integration_test(case_insensitive_factorial);

  type("ACCEPTANCE TESTS");

  suite("operations");
  acceptance_test(arithmetic_operators);
  acceptance_test(boolean_operators);
  acceptance_test(unary_minus);
  acceptance_test(relation_operators);
  acceptance_test(division);

  suite("types_control");
  acceptance_test(types_control_arithmetic);
  acceptance_test(types_control_relation);
  acceptance_test(types_control_jump_if);

  suite("assignment");
  acceptance_test(assignment_vars);

  suite("conditions");
  acceptance_test(if_single);
  acceptance_test(if_else);
  acceptance_test(if_elseif);
  acceptance_test(if_elseif_else);
  acceptance_test(if_single2x);
  acceptance_test(if_else2x);
  acceptance_test(if_elseif2x);
  acceptance_test(if_elseif_else2x);

  suite("loops");
  acceptance_test(do_while_whithout_body);
  acceptance_test(do_while_with_body);
  acceptance_test(do_while_nested);
  acceptance_test(do_while_empty2x);
  acceptance_test(do_while_with_body2x);
  acceptance_test(do_while_nested2x);

  suite("functions");
  acceptance_test(function_simple);
  acceptance_test(function_simple_with_args);
  acceptance_test(function_local_vars);
  acceptance_test(factorial);

  printf("\n");
  printf("  \e[90mcompleted in \e[32m%.5fs\e[0m\n",
         (double)(clock() - start) / CLOCKS_PER_SEC);
  printf("\n");
  return 0;
}
