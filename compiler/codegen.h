#ifndef _CODEGEN_H_
	#define _CODEGEN_H_
	
	#include "parser.h"
	#include "hashmap.h"

	void compileToASM(FILE* file, Lexeme* head);

#endif