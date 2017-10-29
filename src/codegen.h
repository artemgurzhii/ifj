//
// codegen.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_CODE_H
#define IFJ17_CODE_H

#include "ast.h"
#include "vm.h"

// protos

ifj17_vm_t *
ifj17_gen(ifj17_node_t *node);

#endif /* IFJ17_CODE_H */
