//
//  statement.h
//  CHeckScript
//
//  Created by Mashpoe on 6/9/19.
//

#ifndef statement_h
#define statement_h

#include "expression.h"
#include "hashmap.h"

typedef enum heck_stmt_type {
	STMT_EXPR,
	STMT_LET,
	STMT_IF,
	STMT_RET,
	STMT_CLASS,
	STMT_BLOCK,
	STMT_ERR
} heck_stmt_type;

typedef struct heck_stmt {
	heck_stmt_type type;
	void* value;
} heck_stmt;

// EXPRESSION STATEMENT
// just use a regular heck_expr* for expression statements
heck_stmt* create_stmt_expr(heck_expr* expr);

// LET STATEMENT
typedef struct heck_stmt_let {
	string name;
	heck_expr* value;
} heck_stmt_let;
heck_stmt* create_stmt_let(string name, heck_expr* value);

// BLOCK OF CODE
// not a child scope, but has its own child scope and is used in functions
typedef enum { BLOCK_DEFAULT = 0, BLOCK_BREAKS, BLOCK_RETURNS, BLOCK_CAN_RETURN } heck_block_type;
typedef struct heck_block {
	heck_block_type type;
	struct heck_scope* scope;
	heck_stmt** stmt_vec;
} heck_block;
heck_block* create_block(void);
heck_stmt* create_stmt_block(heck_block* block);

// IF STATEMENT
typedef struct heck_stmt_if {
	heck_block_type type;
	heck_expr* condition; // null if this is an "else" block
	heck_block* code;
	struct heck_stmt_if* next; // linked list for if/else ladder
} heck_stmt_if;
heck_stmt_if* create_if_struct(heck_expr* condition);
heck_stmt* create_stmt_if(heck_stmt_if* if_stmt);

// FUNCTION
/*typedef struct heck_stmt_func {
	heck_param** param_vec;
	heck_stmt** stmt_vec;
	
	heck_data_type return_type;
} heck_stmt_func;
heck_stmt* create_stmt_func(void);*/
heck_stmt* create_stmt_ret(heck_expr* value);
// define a function declaration as a vector of overloads for a function of a given name
//typedef heck_stmt_func** heck_func_dec;

// CLASS
typedef struct heck_stmt_class {
	heck_idf name;
	
	// private & public variables
	map_t vars;
} heck_stmt_class;
heck_stmt* create_stmt_class(heck_idf name);

// NAMESPACE
// not to be confused with the scope namespace, heck_scope.
// stores a name so statements can be compiled in the right context
typedef struct heck_stmt_nmsp {
	heck_idf name;
	heck_stmt** stmt_vec;
} heck_stmt_nmsp;
heck_stmt* create_stmt_nmsp(heck_idf name);

// ERROR
heck_stmt* create_stmt_err(void);

void print_stmt(heck_stmt* stmt, int indent);
void print_block(heck_block* block, int indent);

#endif /* statement_h */
