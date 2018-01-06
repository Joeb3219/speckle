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

}

void compileToASM(FILE* file, Lexeme* head){
	Lexeme* child = head->firstChild;
	while(child != NULL){
		// Processes child
		printNode(child);

		// Move to next child
		child = child->nextSibling;
	}
}

