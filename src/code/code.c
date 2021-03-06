//
//  code.c
//  CHeckScript
//
//  Created by Mashpoe on 3/13/19.
//

#include "code.h"
#include "code_impl.h"
#include "scope.h"
#include "str.h"
#include "print.h"
#include <stdio.h>

heck_code* heck_create() {
	heck_code* c = malloc(sizeof(heck_code));
	c->token_vec = vector_create();
	
	heck_scope* block_scope = scope_create(NULL);
	block_scope->namespace = block_scope; // global namespace = global scope
	c->global = block_create(block_scope);
	
	c->strings = str_table_create();
	c->types = type_table_create();
	return c;
}

void free_tokens(heck_code* c) {
	size_t num_tokens = vector_size(c->token_vec);
	for (int i = 0; i < num_tokens; ++i) {
		heck_free_token_data(c->token_vec[i]);
		free(c->token_vec[i]);
	}
}

void heck_free(heck_code* c) {
	free_tokens(c);
	vector_free(c->token_vec);
	str_table_free(c->strings);
	type_table_free(c->types);
	free(c);
}

void heck_print_tokens(heck_code* c) {
	
	int ln = 0;
	
	int indent = 0;
	
	size_t num_tokens = vector_size(c->token_vec);
	for (int i = 0; i < num_tokens; ++i) {
		
		if (c->token_vec[i]->type == TK_BRAC_L) {
			indent++;
		} else if (c->token_vec[i]->type == TK_BRAC_R) {
			indent--;
		}
		
		if (c->token_vec[i]->ln > ln) {
			
			while (ln < c->token_vec[i]->ln) {
				ln++;
				printf("\n% 3d| ", ln);
			}
			
			print_indent(indent);
			
		}
		
		heck_print_token(c->token_vec[i]);
	}
	
	printf("\n");
}

bool heck_add_token(heck_code* c, heck_token* tk) {
	
	vector_add(&c->token_vec, tk);
	
	return true;
}
