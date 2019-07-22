//
//  types.h
//  CHeckScript
//
//  Created by Mashpoe on 3/19/19.
//

#ifndef types_h
#define types_h

typedef enum heck_data_type {
	TYPE_NUM = 0,
	TYPE_STR,
	TYPE_BOOL,
	
	// instance of a specific class, store alongside a heck_idf (may be a child class e.g. ClassA.ClassB)
	TYPE_OBJ, // set the heck_idf to NULL if it is an 'Object' type
	
	TYPE_GEN,		// generic type (type hasn't been deduced by the compiler yet)
	TYPE_VOID,		// void return value
} heck_data_type;

typedef enum heck_literal_type {
	LITERAL_NUM,
	LITERAL_STR,
	LITERAL_TRUE,
	LITERAL_FALSE,
	LITERAL_NULL
} heck_literal_type;

#endif /* types_h */