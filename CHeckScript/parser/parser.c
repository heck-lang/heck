//
//  parser.c
//  CHeckScript
//
//  Created by Mashpoe on 3/26/19.
//

#include "parser.h"
#include "code_impl.h"
#include "expression.h"
#include "statement.h"
#include "scope.h"
#include "tokentypes.h"

#include <stdio.h>

typedef struct parser parser;

struct parser {
	int pos;
	heck_code* code;
	bool success; // true unless there are errors in the code
};

heck_token* step(parser* p) {
	return p->code->token_vec[++p->pos];
}

heck_token* peek(parser* p) {
	return p->code->token_vec[p->pos];
}

heck_token* previous(parser* p) {
	return p->code->token_vec[p->pos-1];
}

bool atEnd(parser* p) {
	return peek(p)->type == TK_EOF;
}

bool match(parser* p, heck_tk_type type) {
	
	if (peek(p)->type == type) {
		step(p);
		return true;
	}
	
	return false;     
}

// don't step because newlines aren't tokens
bool match_endl(parser* p) {
	if (previous(p)->ln < peek(p)->ln) {
		return true;
	}
	return false;
}

void panic_mode(parser* p) {
	// we are in panic mode, so obviously the code won't compile
	p->success = false;
	
	// step until we are in a new statement
	for (;;) {
		if (atEnd(p)) {
			return;
		}
		switch (peek(p)->type) {
			case TK_BRAC_L:
			case TK_BRAC_R:
			case TK_KW_LET:
			case TK_KW_IF:
			case TK_KW_FUNC:
			case TK_KW_CLASS:
				return;
			default:
				step(p);
				break;
		}
	}
}

/*
 *
 * Parsing Expressions
 *
 */

// forward declarations:
heck_expr* expression(parser* p);

heck_idf identifier(parser* p) { // assumes an identifier was just found with match(p)
	
	int len = 0, alloc = 1;
	string* idf = malloc(sizeof(string) * (alloc + 1));
	
	do {
		// add string to identifier chain
		idf[len++] = previous(p)->value;
		// reallocate if necessary
		if (len == alloc) {
			idf = realloc(idf, sizeof(string) * (++alloc + 1));
		}

		if (!match(p, TK_OP_DOT))
			break;

	} while (match(p, TK_IDF));

	// add null terminator
	idf[len] = NULL;

	return idf;
}

heck_expr* primary(parser* p) {
	if (match(p, TK_KW_FALSE)) return create_expr_literal(NULL, TK_KW_FALSE);
	if (match(p, TK_KW_TRUE)) return create_expr_literal(NULL, TK_KW_TRUE);
	//if (match(NIL)) return new Expr.Literal(null);
	
	if (match(p, TK_NUM) || match(p, TK_STR)) {
		return create_expr_literal(previous(p)->value, previous(p)->type);
	}
	
	// TODO: add support for "global" keyword, e.g. global.xyz = "value"
	// This is the ONLY place where the global keyword should be used
	if (match(p, TK_IDF)) {
		
		heck_idf name = identifier(p);
		
		if (match(p, TK_PAR_L)) { // function call
			heck_expr* call = create_expr_call(name);
			
			if (match(p, TK_PAR_R))
				return call;
			
			for (;;) {
				_vector_add(&((heck_expr_call*)call->expr)->arg_vec, heck_expr*) = expression(p);
				
				if (match(p, TK_PAR_R)) {
					return call;
				} else if (!match(p, TK_COMMA)) {
					// TODO: report expected ')'
					panic_mode(p);
					break;
				}
			}
			
			return call;
			
		} else { // variable?
			return create_expr_value(name);
		}
	}
	
	if (match(p, TK_PAR_L)) { // parentheses grouping
		heck_expr* expr = expression(p);
		if (match(p, TK_PAR_R)) {
			return expr;
		} else {
			// TODO: report expected ')'
		}
		
		
		//Expr expr = expression();
		//consume(RIGHT_PAREN, "Expect ')' after expression.");
		//return new Expr.Grouping(expr);
	}
	
	// TODO: report expected expression
	panic_mode(p);
	return create_expr_err();
}

heck_expr* unary(parser* p) {
	if (match(p, TK_OP_NOT) || match(p, TK_OP_SUB)) {
		heck_tk_type operator = previous(p)->type;
		heck_expr* expr = unary(p);
		return create_expr_unary(expr, operator);
	}
	
	return primary(p);
}

heck_expr* multiplication(parser* p) {
	heck_expr* expr = unary(p);
	
	while (match(p, TK_OP_MULT) || match(p, TK_OP_DIV) || match(p, TK_OP_MOD)) {
		heck_tk_type operator = previous(p)->type;
		heck_expr* right = unary(p);
		expr = create_expr_binary(expr, operator, right);
	}
	
	return expr;
}

heck_expr* addition(parser* p) {
	heck_expr* expr = multiplication(p);
	
	while (match(p, TK_OP_ADD) || match(p, TK_OP_SUB)) {
		heck_tk_type operator = previous(p)->type;
		heck_expr* right = multiplication(p);
		expr = create_expr_binary(expr, operator, right);
	}
	
	return expr;
}

heck_expr* comparison(parser* p) {
	heck_expr* expr = addition(p);
	
	while (match(p, TK_OP_GT) || match(p, TK_OP_GT_EQ) || match(p, TK_OP_LESS) || match(p, TK_OP_LESS_EQ)) {
		heck_tk_type operator = previous(p)->type;
		heck_expr* right = addition(p);
		expr = create_expr_binary(expr, operator, right);
	}
	
	return expr;
}

heck_expr* equality(parser* p) {
	heck_expr* expr = comparison(p);
	
	while (match(p, TK_OP_EQ) || match(p, TK_OP_N_EQ)) {
		heck_tk_type operator = previous(p)->type;
		heck_expr* right = comparison(p);
		expr = create_expr_binary(expr, operator, right);
	}
	
	return expr;
}

heck_expr* assignment(parser* p) {
	heck_expr* expr = equality(p);
	
	if (match(p, TK_OP_ASG)) {
		
		if (expr->type == EXPR_VALUE) {
			return create_expr_asg((heck_idf)expr->expr, expression(p));
		}
		
		// TODO: report invalid assignment target
	}
	
	return expr;
}

heck_expr* ternary(parser* p) {
	heck_expr* expr = assignment(p);
	
	if (match(p, TK_Q_MARK)) {
		heck_expr* value_a = expression(p);
		
		if (!match(p, TK_COLON)) {
			// TODO: report expected ':'
			return create_expr_ternary(expr, value_a, create_expr_err());
		} else {
			return create_expr_ternary(expr, value_a, expression(p));
		}
		
	}
	
	return expr;
}

heck_expr* expression(parser* p) {
	return ternary(p);
}

/*
 *
 * Parsing Statements
 *
 */

// forward declarations
heck_stmt* statement(parser* p, heck_scope* scope);

heck_stmt* let_statement(parser* p) {
	step(p);
	
	if (match(p, TK_IDF)) {
		string name = previous(p)->value;
		if (match(p, TK_OP_ASG)) { // =
			return create_stmt_let(name, expression(p));
		} else {
			// TODO: report expected '='
		}
	} else {
		// TODO: report expected identifier
	}
	panic_mode(p);
	return create_stmt_err();
}

heck_stmt* if_statement(parser* p, heck_scope* scope) {
	step(p);
	
	// if statements do not need (parentheses) around the condition in heck
	heck_stmt* s = create_stmt_if(expression(p));
	if (match(p, TK_BRAC_L)) {
		
		for (;;) {
			if (atEnd(p)) {
				// TODO report unexpected EOF
				break;
			} else if (match(p, TK_BRAC_R)) {
				break;
			} else {
				_vector_add(&((heck_stmt_if*)s->value)->stmt_vec, heck_stmt*) = statement(p, scope);
			}
		}
		
	} else {
		// TODO: report expected '{'
		
		// populate if statement with only an error
		_vector_add(&((heck_stmt_if*)s->value)->stmt_vec, heck_stmt*) = create_stmt_err();
		
		panic_mode(p);
	}
	return s;
}

// TODO: error if there are any duplicate parameter names
void func_statement(parser* p, heck_scope* scope) {
	step(p);
	
	if (!match(p, TK_IDF)) {
		// TODO: report expected identifier
		panic_mode(p);
		return;
	}
	
	heck_func* func = create_func();
	heck_scope* child = scope_add_func(scope, func, identifier(p));
	
	if (!match(p, TK_PAR_L)) {
		// TODO: report expected '('
		panic_mode(p);
		return;
	}
	
	// parse parameters
	if (!match(p, TK_PAR_R)) {
		
		for (;;) {
			
			if (!match(p, TK_IDF)) {
				// TODO: report expected an identifier
				panic_mode(p);
				return;
			}
			
			// create the parameter
			heck_idf param_type = NULL;
			heck_idf param_name = identifier(p);
			
			if (match(p, TK_IDF)) {
				param_type = param_name;
				param_name = identifier(p);
			}
			
			
			if (param_name[1] != NULL) { // if element[1] is null than the identifier has one value
				// TODO: report invalid parameter name (must not contain '.' separated values)
				
				if (param_type != NULL) {
					free((void*)param_type);
				}
				free((void*)param_name);
				return;
			}
			
			heck_param* param = create_param(param_name[0]);
			free((void*)param_name);
			
			if (param_type != NULL) {
				param->type = TYPE_OBJ;
				param->obj_type = param_type;
			}
			
			if (match(p, TK_OP_ASG)) { // handle default argument values (e.g. arg = expr)
				param->def_val = expression(p);
			}
			
			_vector_add(&func->param_vec, heck_param*) = param;
			
			// continue if there is a comma
			if (!match(p, TK_COMMA)) {
				
				if (match(p, TK_PAR_R)) {
					break; // stop the loop
				}
				// TODO: report expected ')'
				panic_mode(p);
				return;
			}
		}
	}
	
	if (match(p, TK_BRAC_L)) {
		
		for (;;) {
			if (atEnd(p)) {
				// TODO report unexpected EOF
				break;
			} else if (match(p, TK_BRAC_R)) {
				break;
			} else {
				_vector_add(&func->stmt_vec, heck_stmt*) = statement(p, child);
			}
		}
		
	} else {
		// TODO: report expected '{'
		
		// populate function with only an error
		//_vector_add(&func->stmt_vec, heck_stmt*) = create_stmt_err();
		
		panic_mode(p);
	}
	
	return;
	
}

heck_stmt* ret_statement(parser* p) {
	step(p);
	
	// expression must start on the same line as return statement or else it's void
	if (match(p, TK_SEMI) || match_endl(p)) {
		return create_stmt_ret(NULL);
	}
	
	return create_stmt_ret(expression(p));
}

heck_stmt* scope_statement(parser* p, heck_scope* scope) {
	step(p);
	
	heck_stmt* s = create_stmt_scope();
	
	for (;;) {
		if (atEnd(p)) {
			// TODO report unexpected EOF
			break;
		} else if (match(p, TK_BRAC_R)) {
			break;
		} else {
			_vector_add(&s->value, heck_stmt*) = statement(p, scope);
		}
	}
	
	return s;
}

heck_stmt* namespace(parser* p, heck_scope* scope) {
	
	// check if namespace is already defined
	
	return NULL;
}

heck_stmt* statement(parser* p, heck_scope* scope) {
	
	switch (peek(p)->type) {
		case TK_KW_LET:
			return let_statement(p);
			break;
		case TK_KW_IF:
			return if_statement(p, scope);
			break;
		case TK_KW_FUNC:
			func_statement(p, scope);
			return NULL;
			break;
		case TK_KW_RETURN:
			return ret_statement(p);
			break;
		case TK_BRAC_L:
			return scope_statement(p, scope);
			break;
		default:
			return create_stmt_expr(expression(p));
	}
}

bool heck_parse(heck_code* c) {
	
	parser* p = malloc(sizeof(parser));
	p->pos = 0;
	p->code = c;
	p->success = true;
	
	//heck_namespace* global = create_namespace(NULL);
	
	while (!atEnd(p)) {
		heck_stmt* e = statement(p, c->global);
//
//		switch(e->type) {
//			case STMT_LET:
//
//				hashmap_put(global->var_map, ((heck_stmt_let*)e->value)->name, e->value);
//				break;
//			case STMT_FUNC: {
//				hashmap_put(c->global->scope_map, ((heck_stmt_func*)e->value)->name, e->value);
//				break;
//			}
//			default:
//				break;
//
//		}
		if (e) {
			print_stmt(e, 0);
		}
		//_vector_add(c->syntax_tree_vec, heck_stmt*) = statement(p);
	}
	
	print_scope(c->global);
	
	return p->success;
}
