//
// codegen.c
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#include "ast.h"
#include "codegen.h"
#include "internal.h"
#include "opcodes.h"
#include "visitor.h"
#include <stdio.h>

// TODO: MSB
#define CONST(val)                                                                  \
  (vm->main->constants[vm->main->nconstants] = val, 32 + vm->main->nconstants++)

/*
 * Emit an instruction.
 */

#define emit(op, a, b, c) *vm->main->code++ = ABC(op, a, b, c);

//BINARY_OP
static int bin_op = 0;
static int rel = 0;

//IF
static int from_if = 0;
static int else_if_num = 0;
static int else_num = 0;
static int end_if_num = 0;

//FUNCTION
static int args = 0;
static int params = 0;
static int from_func = 0;
static int from_call = 0;
static int from_return = 0;
static int glob_var = 0;
static int loc_var = 0;
static int scope = 0;

// LOOPS
static int from_loop = 0;
static int loop_num = 0;
static int mem_loop_num = 0;


// print function

int (*print_func)(const char *format, ...);

void ifj17_set_codegenprint_func(int (*func)(const char *format, ...)) {
  print_func = func;
}

/*
 * Emit binary operation.
 */

static void emit_op(ifj17_visitor_t *self, ifj17_binary_op_node_t *node) {
  switch (node->op) {
  case IFJ17_TOKEN_OP_PLUS:
    print_func("ADD ");
    break;

  case IFJ17_TOKEN_OP_MINUS:
     print_func("SUB ");
     break;

  case IFJ17_TOKEN_OP_MUL:
       print_func("MUL ");
       break;
  case IFJ17_TOKEN_OP_DIV:
        print_func("DIV ");
        break;

  case IFJ17_TOKEN_OP_EQ:
     print_func("JUMPIFEQ ");
      if (from_if == 1) {
        print_func("RES_IF_%d ", else_if_num);
        from_if--;
      }
     break;

  case IFJ17_TOKEN_OP_NEQ:
     print_func("JUMPIFNEQ ");
      if (from_if == 1) {
        print_func("RES_IF_%d ", else_if_num);
        from_if--;
      }
     break;

  case IFJ17_TOKEN_OP_BIT_AND:
     print_func("AND ");
     break;

  case IFJ17_TOKEN_OP_BIT_OR:
     print_func("OR ");
     break;

  case IFJ17_TOKEN_OP_LNOT:
     print_func("NOT ");
     break;
  // case IFJ17_TOKEN_OP_MOD:
  //   emit(MOD, 0, l, r);
  //   break;
  // case IFJ17_TOKEN_OP_LT:
  //   emit(LT, 0, l, r);
  //   emit(JMP, 0, 1, 0);
  //   emit(LOADB, 0, CONST(1), 1);
  //   emit(LOADB, 0, CONST(0), 0);
  //   break;
  // case IFJ17_TOKEN_OP_LTE:
  //   emit(LTE, 0, l, r);
  //   emit(JMP, 0, 1, 0);
  //   emit(LOADB, 0, CONST(1), 1);
  //   emit(LOADB, 0, CONST(0), 0);
  //   break;
  }

    if (from_loop == 1) {
       print_func("LOOP_%d ", loop_num--);
      from_loop--;
  }
}

/*

0: lt 5 2;
 : false1:
 : ...
 : ...
 : true1:
 : ...
 : ...
 : endif1:
 : ...
 : ...
 : ...
*/

/*
 * Visit block `node`.
 */

static void visit_block(ifj17_visitor_t *self, ifj17_block_node_t *node) {
  ifj17_vec_each(node->stmts, { visit((ifj17_node_t *)val->value.as_pointer); });
}

/*
 * Visit int `node`.
 */

static void visit_int(ifj17_visitor_t *self, ifj17_int_node_t *node) {
  if(from_return == 1) {
    printf("PUSHS ");
    print_func("int@%d", node->val);
    from_return--;
  }
  else {
    print_func("int@%d", node->val);
  }
}

/*
 * Visit double `node`.
 */

static void visit_double(ifj17_visitor_t *self, ifj17_double_node_t *node) {
  ifj17_vm_t *vm = (ifj17_vm_t *)self->data;
}

/*
 * Visit id `node`.
 */

static void visit_id(ifj17_visitor_t *self, ifj17_id_node_t *node) {
  //printf("from_return= %d && from_func= %d\n", from_return, from_func);
//printf("from_func= %d && from_return= %d\n", from_func, from_return);
//printf("from_call= %d\n", from_call);
  if (from_call == 1) {
    print_func("%s", node->val);
    from_call--;
  }
  else if (from_func == 1 && from_return == 0) {
    print_func("TF@%s", node->val);
      if(args) {
        printf("\n");
      }
  }

  else if(from_return == 1) {
    printf("PUSHS ");
    print_func("TF@%s", node->val);
    from_return--;
  }
  else {
    print_func("GF@%s", node->val);
      if (args) {
        print_func("\n", node->val);
      }
  }
}

/*
 * Visit decl `node`.
 */

static void visit_decl(ifj17_visitor_t *self, ifj17_decl_node_t *node) {
  // printf("(decl %s:%s", node->base, node->type ? node->type : "");
  // if (node->vec) visit(node->vec);
  // printf(")");

  // printf("I AM IN DECL\n");
  // if (from_func == 1) {
  //   print_func("CREATEFRAME\n");
  // }
  ifj17_vec_each(node->vec, {

    if (params == 1) {
      print_func("DEFVAR ");
      visit((ifj17_node_t *)val->value.as_pointer);
      print_func("\n");

      printf("POPS ");
      visit((ifj17_node_t *)val->value.as_pointer);
      print_func("\n");

    }
    else {
      print_func("DEFVAR ");
      visit((ifj17_node_t *)val->value.as_pointer);
      print_func("\n");
    }
  });

  // if (node->type) {
  //   visit(node->type);
  // }

  // printf("GOING OUT OF DECL\n");
}

/*
 * Visit string `node`.
 */

static void visit_string(ifj17_visitor_t *self, ifj17_string_node_t *node) {
  print_func("\"%s\"", node->val);
}

/*
 * Visit unary op `node`.
 */

static void visit_unary_op(ifj17_visitor_t *self, ifj17_unary_op_node_t *node) {
  ifj17_vm_t *vm = (ifj17_vm_t *)self->data;
  visit(node->expr);
  emit(NEGATE, 0, 0, 0);
}

/*
 * Visit binary op `node`.
 */

static void visit_binary_op(ifj17_visitor_t *self, ifj17_binary_op_node_t *node) {
  // ifj17_vm_t *vm = (ifj17_vm_t *)self->data;
  // if (IFJ17_NODE_BINARY_OP == node->left->type) {
  //   visit(node->left);
  //   int r = CONST(((ifj17_double_node_t *)node->right)->val);
  //   emit_op(vm, node, 0, r);
  // } else {
  //   int l = CONST(((ifj17_double_node_t *)node->left)->val);
  //   int r = CONST(((ifj17_double_node_t *)node->right)->val);
  //   emit_op(vm, node, l, r);
  // }

  // printf("%s ", ifj17_token_type_string(node->op));
    // switch (ifj17_token_type_string(node->op)) {
    //   case '=':
    //     printf("MOVE ");
    // } // SWITCH NOT WORKING BECAUSE OF STRING FUCK C

    // REMOVE REPEATABLE CODE!!!!

    if (!strcmp(ifj17_token_type_string(node->op), "=")) {
      if (node->right->type != IFJ17_NODE_BINARY_OP) {

        if (node->right->type == IFJ17_NODE_CALL) {
            visit(node->right);
            print_func("\n");
            print_func("POPS ");
            visit(node->left);
            print_func("\n");
            return;
          }

        else if (node->right->type == IFJ17_NODE_UNARY_OP) {
            print_func("NOT ");
            visit(node->left);
            print_func(" ");
            visit(node->right);
            print_func("\n");
            return;
          }

        else {
            print_func("MOVE ");
            visit(node->left);
            print_func(" ");
            visit(node->right);
            print_func("\n");
            return;
          }
        }
      }

    //emit_op(self, node);

    if (!strcmp(ifj17_token_type_string(node->op), "+") ||
      !strcmp(ifj17_token_type_string(node->op), "-") ||
      !strcmp(ifj17_token_type_string(node->op), "*") ||
      !strcmp(ifj17_token_type_string(node->op), "/") ||
      !strcmp(ifj17_token_type_string(node->op), "and") ||
      !strcmp(ifj17_token_type_string(node->op), "or") ) {

        if (bin_op == 1) {
          emit_op(self, node);
          return;
        } else if (bin_op == 2) {
          visit(node->left);
          print_func(" ");
          visit(node->right);
          print_func("\n");
          bin_op = bin_op - 2;
          return;
        }
      }


     else if (!strcmp(ifj17_token_type_string(node->op), "==")) {
       emit_op(self, node);
       visit(node->left);
       print_func(" ");
       visit(node->right);
       print_func("\n");
       return;
     }

     else if (!strcmp(ifj17_token_type_string(node->op), "<>")) {
       emit_op(self, node);
       visit(node->left);
       print_func(" ");
       visit(node->right);
       print_func("\n");
       return;
     }

     else if (!strcmp(ifj17_token_type_string(node->op), ">")) {
       printf("CREATEFRAME\n");
       printf("DEFVAR TF@temp_bool\n");
       printf("GT TF@temp_bool ");
       visit(node->left);
       printf(" ");
       visit(node->right);
       printf("\n");
       printf("JUMPIFEQ RES_IF_%d TF@temp_bool ", else_if_num);
       printf("bool@true");
       print_func("\n");
       return;
     }

     else if (!strcmp(ifj17_token_type_string(node->op), "<")) {
       printf("CREATEFRAME\n");
       printf("DEFVAR TF@temp_bool\n");
       printf("LT TF@temp_bool ");
       visit(node->left);
       printf(" ");
       visit(node->right);
       printf("\n");
       printf("JUMPIFEQ RES_IF_%d TF@temp_bool ", else_if_num);
       printf("bool@true");
       print_func("\n");
       return;
     }


    bin_op++;
    visit(node->right);
    visit(node->left);
    print_func(" ");
    bin_op++;
    visit(node->right);
  }


/*
 * Visit array `node`.
 */

static void visit_array(ifj17_visitor_t *self, ifj17_array_node_t *node) {
  // printf("(array\n");
  // ++indents;
  // ifj17_vec_each(node->vals, {
  //   INDENT;
  //   visit((ifj17_node_t *) val->value.as_pointer);
  //   if (i != len - 1) printf("\n");
  // });
  // --indents;
  // printf(")");
}

/*
 * Visit hash `node`.
 */

static void visit_hash(ifj17_visitor_t *self, ifj17_hash_node_t *node) {
  // printf("(hash\n");
  // ++indents;
  // ifj17_hash_each(node->vals, {
  //   INDENT;
  //   printf("%s: ", slot);
  //   visit((ifj17_node_t *) val->value.as_pointer);
  //   printf("\n");
  // });
  // --indents;
  // printf(")");
}

/*
 * Visit subscript `node`.
 */

static void visit_subscript(ifj17_visitor_t *self, ifj17_subscript_node_t *node) {}

/*
 * Visit slot `node`.
 */

static void visit_slot(ifj17_visitor_t *self, ifj17_slot_node_t *node) {
  // printf("(slot\n");
  // ++indents;
  // INDENT;
  // visit(node->left);
  // printf("\n");
  // INDENT;
  // visit(node->right);
  // --indents;
  // printf(")");
}

/*
 * Visit call `node`.
 */

static void visit_call(ifj17_visitor_t *self, ifj17_call_node_t *node) {


  args = ifj17_vec_length(node->args->vec);

  if (ifj17_vec_length(node->args->vec)) {
    print_func("PUSHS ");

    ifj17_vec_each(node->args->vec, {
      visit((ifj17_node_t *)val->value.as_pointer); });
      args = 0;
    }
  if (scope != 1) {
    printf("PUSHFRAME\n");
  }
  print_func("CALL ");
  from_call++;
  visit((ifj17_node_t *)node->expr);

  if (scope != 1) {
    printf("\nPOPFRAME");
  }
}

/*
 * Visit scope `node`.
 */

static void visit_scope(ifj17_visitor_t *self, ifj17_scope_node_t *node) {
  print_func("LABEL Scope\n");
  scope++;
  visit((ifj17_node_t *)node->block);
  scope--;
  // print_func("(scope %s -> ");
  // ++indents;
  //
  // --indents;
  // print_func("\n");
  // ++indents;
  // visit((ifj17_node_t *)node->block);
  // --indents;
  // print_func(")");
}

/*
 * Visit dim `node`.
 */

static void visit_dim(ifj17_visitor_t *self, ifj17_dim_node_t *node) {
  // printf("I AM IN DIM\n");
  if (from_func == 1) {
    loc_var++;
  }
  ifj17_vec_each(node->vec, {
    ifj17_binary_op_node_t *bin = (ifj17_binary_op_node_t *)val->value.as_pointer;

    visit(bin->left);

    // if (bin->right) {
    //
    //   visit(bin->right);
    // }
  });

  // printf("GOING OUT OF DIM\n");

}

/*
 * Visit function `node`.
 */

static void visit_function(ifj17_visitor_t *self, ifj17_function_node_t *node) {
  from_func++;
  printf("LABEL ");
  print_func("%s\n", node->name);
  printf("CREATEFRAME \n");

      ifj17_vec_each(node->params, {
        params++;
        visit((ifj17_node_t *)val->value.as_pointer);
        params--;
      });

  visit((ifj17_node_t *)node->block);

  // printf("(function %s -> %s", node->name, node->type ? node->type : "");
  // ++indents;
  // ifj17_vec_each(node->params, {
  //   printf("\n");
  //   INDENT;
  //   visit((ifj17_node_t *) val->value.as_pointer);
  // });
  // --indents;
  // printf("\n");
  // ++indents;
  // visit((ifj17_node_t *) node->block);
  // --indents;
  // printf(")");
  from_func--;
}

/*
 * Visit `while` node.
 */

 static void visit_while(ifj17_visitor_t *self, ifj17_while_node_t *node) {
   loop_num = mem_loop_num;
    printf("LABEL LOOP_%d\n", ++loop_num);
   mem_loop_num++;
   visit((ifj17_node_t *) node->block);

   from_loop++;
   //printf("JUMPIF");
   visit((ifj17_node_t *) node->expr);
}

/*
 * Visit `return` node.
 */

static void visit_return(ifj17_visitor_t *self, ifj17_return_node_t *node) {

  if (node->expr) {
    from_return++;
    visit((ifj17_node_t *)node->expr);
    print_func(" \n");
  }
  print_func("RETURN\n");
  //printf("from_return_visit= %d\n", from_return);
  // if (node->expr) {
  //   ++indents;
  //   printf("\n");
  //   INDENT;
  //   visit((ifj17_node_t *) node->expr);
  //   --indents;
  // }
  // printf(")");
  //from_func--;
}

/*
 * Visit if `node`.
 */

static void visit_if(ifj17_visitor_t *self, ifj17_if_node_t *node) {
  // We need these counters to make LABELs in bytecode with
  // unique name. Should be refactored later if we have enough time

  int mem_else_if_num = else_if_num;
  else_if_num++;
  from_if++;
  end_if_num++;

  // if
//   if (rel != 1) {
//   print_func("JUMPIF");
// }

  emit_op(self, node);
  visit((ifj17_node_t *)node->expr);


  // else ifs
  ifj17_vec_each(node->else_ifs, {
    from_if++;
    else_if_num++;
    ifj17_if_node_t *else_if = (ifj17_if_node_t *)val->value.as_pointer;
    //print_func("JUMPIF");
    visit((ifj17_node_t *)else_if->expr);
  });

  // else
  if (node->else_block) {
    print_func("JUMP RES_ELSE_%d\n", ++else_num);
  }

  if (!node->else_block && ifj17_vec_length(node->else_ifs) == 0) {
    print_func("JUMP END_IF_%d\n", end_if_num);
  }

  print_func("LABEL RES_IF_%d\n", ++mem_else_if_num);

  visit((ifj17_node_t *)node->block);

  print_func("JUMP END_IF_%d\n", end_if_num);

  // else ifs
  ifj17_vec_each(node->else_ifs, {
    ifj17_if_node_t *else_if = (ifj17_if_node_t *)val->value.as_pointer;
    print_func("LABEL RES_IF_%d\n", ++mem_else_if_num);
    visit((ifj17_node_t *)else_if->block);
    print_func("JUMP END_IF_%d\n", end_if_num);
  });

  if (node->else_block) {
    print_func("LABEL RES_ELSE_%d\n", else_num);
    visit((ifj17_node_t *)node->else_block);
    print_func("JUMP END_IF_%d\n", end_if_num);
  }

  print_func("LABEL END_IF_%d\n", end_if_num);

}

/*
 * Visit `type` node.
 */

static void visit_type(ifj17_visitor_t *self, ifj17_type_node_t *node) {}

/*
 * Generate code for the given `node`.
 */

ifj17_vm_t *ifj17_gen(ifj17_node_t *node) {
  ifj17_vm_t *vm = malloc(sizeof(ifj17_vm_t));
  if (!vm)
    return NULL;
  vm->main = malloc(sizeof(ifj17_activation_t));
  vm->main->nconstants = 0;
  vm->main->constants = malloc(1024 * sizeof(int)); // TODO: vec / objects
  vm->main->ip = vm->main->code = malloc(64 * 1024);
  ifj17_visitor_t visitor = {.data = (void *)vm,
                             .visit_if = visit_if,
                             .visit_id = visit_id,
                             .visit_int = visit_int,
                             .visit_slot = visit_slot,
                             .visit_call = visit_call,
                             .visit_hash = visit_hash,
                             .visit_array = visit_array,
                             .visit_while = visit_while,
                             .visit_block = visit_block,
                             .visit_dim = visit_dim,
                             .visit_decl = visit_decl,
                             .visit_double = visit_double,
                             .visit_string = visit_string,
                             .visit_return = visit_return,
                             .visit_function = visit_function,
                             .visit_scope = visit_scope,
                             .visit_unary_op = visit_unary_op,
                             .visit_binary_op = visit_binary_op,
                             .visit_subscript = visit_subscript,
                             .visit_type = visit_type};

  print_func(".IFJcode17\n");
  print_func("JUMP Scope\n");
  ifj17_visit(&visitor, node);

  // Reset code so we can free it later
  vm->main->code = vm->main->ip;
  print_func("\n");
  return vm;
}
