#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "speckle.h"
#include "error.h"
#include "parser.h"
#include "lexer.h"
#include "hashmap.h"

// Private function declarations
void compileFunction(FILE* file, Lexeme* function);



void compileFunction(FILE* file, Lexeme* function){
	// Ensure that we are dealing with a function declaration
	if(function->type != LEX_FUNC) ERR_UNEXPECTED_LEXEME_EXPECTED(function, LEX_FUNC);
}

void compileToASM(FILE* file, Lexeme* head){
	Lexeme* child = head->firstChild;
	while(child != NULL){
		// Processes child iff it has a child underneath it
		// If there is nothing underneath it, then there isn't 
		// any code, ie it's an empty line.
		if(child->firstChild != NULL){
			compileFunction(file, child->firstChild);
		}

		// Move to next child
		child = child->nextSibling;
	}
}

