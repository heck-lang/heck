//
//  function.h
//  Heck
//
//  Created by Mashpoe on 7/31/19.
//

#ifndef function_h
#define function_h

#include <stdio.h>
#include "statement.h"
#include "str.h"
#include "types.h"
#include "declarations.h"

typedef struct heck_scope heck_scope;

// list of overloads for a function with a given name
typedef struct heck_func_list {
	heck_func** func_vec;
} heck_func_list;

// TODO: rename
// instance of a generic function
typedef struct heck_func_gen_list {
	heck_type_arg_list type_args;
	void* func_code; // the code produced from compiling the function using the types from call
} heck_func_gen_inst;

// use call_gen_vec for generic functions, use func_type for functions with set parameter types
typedef union heck_func_value {
	heck_func_gen_inst** gen_inst_vec;
	void* func_type;
} heck_func_value;

// FUNCTION PARAMETER
typedef struct heck_param {
	str_entry name; // name of the parameter
	
	heck_data_type* type;
	heck_idf obj_type; // NULL unless type == TYPE_CLASS
	
	heck_expr* def_val; // default value
} heck_param;
heck_param* param_create(str_entry name);

// FUNCTION
typedef struct heck_func {
	// TODO: bitmask these bois
	bool declared; // heck_func implied definition, so we just need to check if there is a declaration
	bool generic; // if it's generic, use value.gen_inst_vec
	
	// if param types are generic, a new overload is added to the function's scope
	// each time the function is compiled
	heck_param** param_vec;
	
	heck_func_value value;
	
	heck_block* code;
	
	heck_data_type* return_type;
} heck_func;
heck_func* func_create(heck_scope* parent, bool declared);
void func_free(heck_func* func);

bool func_add_overload(heck_func_list* list, heck_func* func);
heck_scope* scope_add_func(heck_scope* scope, heck_func* func, heck_idf name);

// finds the correct definition/overload for a given call
heck_func* func_match_def(heck_scope* scope, heck_expr_call* call);

// checks if a definition matches a given argument list
// finds the best match with the precedence exact=>generic=>castable
bool func_overload_exists(heck_func_list* list, heck_func* func);

bool func_def_resolve(heck_func* func);

// prints all definitions/declarations for a given function
void print_func_defs(heck_func_list* list, const char* name, int indent);

#endif /* function_h */
