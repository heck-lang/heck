//
//  expression.h
//  CHeckScript
//
//  Created by Mashpoe on 3/19/19.
//

#ifndef expression_h
#define expression_h

#include "types.h"
#include "tokentypes.h"
#include "str.h"
#include "vec.h"

typedef enum heck_expr_type heck_expr_type;
enum heck_expr_type {
	EXPR_BINARY,
	EXPR_UNARY,
	EXPR_LITERAL,
	EXPR_VALUE,		// value of a variable
	EXPR_CALL,
	EXPR_ASG,
	EXPR_TER,
	EXPR_ERR		// error parsing
};

typedef struct heck_expr heck_expr;
struct heck_expr {
	heck_expr_type type;
	void* expr;
};

typedef struct heck_expr_binary heck_expr_binary;
struct heck_expr_binary {
	heck_expr* left;
	heck_tk_type operator;
	heck_expr* right;
};

typedef struct heck_expr_unary heck_expr_unary;
struct heck_expr_unary {
	heck_expr* expr;
	heck_tk_type operator;
};

typedef struct heck_expr_literal heck_expr_literal;
struct heck_expr_literal {
	heck_tk_type type; // TODO ((((((maybe)))))) make it heck_type
	void* value;
};

// vector of consecutive identifiers separated by '.'
typedef const string* heck_idf;

// variable value
typedef heck_idf heck_expr_value;

// function call
typedef struct heck_expr_call heck_expr_call;
struct heck_expr_call {
	heck_idf name;
	heck_expr** arg_vec; // arguments
};

typedef struct heck_expr_asg heck_expr_asg;
struct heck_expr_asg {
	heck_idf name;
	heck_expr* value;
};

typedef struct heck_expr_ternary heck_expr_ternary;
struct heck_expr_ternary {
	heck_expr* condition;
	heck_expr* value_a;
	heck_expr* value_b;
};

heck_expr* create_expr_binary(heck_expr* left, heck_tk_type operator, heck_expr* right);

heck_expr* create_expr_unary(heck_expr* expr, heck_tk_type operator);

heck_expr* create_expr_literal(void* value, heck_tk_type type);

heck_expr* create_expr_value(heck_idf name);

heck_expr* create_expr_call(heck_idf name);

heck_expr* create_expr_asg(heck_idf name, heck_expr* value);

heck_expr* create_expr_ternary(heck_expr* condition, heck_expr* value_a, heck_expr* value_b);

heck_expr* create_expr_err(void);

void free_expr(heck_expr* expr);

void print_idf(heck_idf idf);

void print_expr(heck_expr* expr);

#endif /* expression_h */
