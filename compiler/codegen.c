#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "speckle.h"
#include "error.h"
#include "parser.h"
#include "lexer.h"
#include "hashmap.h"

#define FUNCTION_PREPEND "speckle_fn_"
#define INT_SIZE 8

// Private function declarations
void compileFunction(FILE* file, Lexeme* function, Hashmap* functionsMap);
int findArguments(Lexeme* function, Hashmap* hashmap);
Lexeme* findFirstLexemeOccurence(Lexeme* head, LexemeType type);
void compileStmtlist(FILE* file, Lexeme* stmtlist, Hashmap* variables);


Lexeme* findFirstLexemeOccurence(Lexeme* head, LexemeType type){
	if(head == NULL) return NULL;

	Lexeme* child = head->firstChild;
	Lexeme* result;
	if(head->type == type) return head;

	while(child != NULL){
		result = findFirstLexemeOccurence(child, type);
		if(result != NULL) return result;

		child = child->nextSibling;
	}

	return NULL;
}

int findArguments(Lexeme* function, Hashmap* hashmap){
	int count = 0;
	Lexeme* argsList = findFirstLexemeOccurence(function, LEX_ARGLIST);
	Lexeme* child;

	if(argsList == NULL) return count;	// If there is no args list, we just assume an empty list.

	//printAST("-", argsList);
	int currentPosition = 8;
	
	while(argsList != NULL && argsList->firstChild != NULL){
		child = argsList->firstChild;
		if(child->type != LEX_IDENTIFIER) ERR_UNEXPECTED_LEXEME_EXPECTED(child, LEX_IDENTIFIER);

		hashmapInsert(hashmap, child->token->data, currentPosition);
		currentPosition += INT_SIZE;
		count ++;
		
		argsList = child->nextSibling;
	}

	return count;
}

void findAllVariables(int* current, Lexeme* stmt, Hashmap* variables){
	if(stmt == NULL) return;

	Lexeme* single;
	if(stmt->type == LEX_DECLARATION){
		single = stmt->firstChild;
		if(single->type == LEX_ASSIGN){
			single = single->firstChild;
			if(single->type == LEX_IDENTIFIER){
				// We have found an identifier! Call the press.
				hashmapInsert(variables, single->token->data, *current);
				(*current) -= INT_SIZE;
			}
		}
	}

	findAllVariables(current, stmt->firstChild, variables);

	single = stmt->nextSibling;
	while(single != NULL){
		findAllVariables(current, single, variables);
		single = single->nextSibling;
	}
}

void compileStmtlist(FILE* file, Lexeme* stmtlist, Hashmap* variables){
//	printAST("-", stmtlist);

	fprintf(file, ".body:\n");
}

void compileFunction(FILE* file, Lexeme* function, Hashmap* functionsMap){
	// Ensure that we are dealing with a function declaration
	if(function->type != LEX_FUNC) ERR_UNEXPECTED_LEXEME_EXPECTED(function, LEX_FUNC);
	Hashmap* variableMap = createHashmap();

	int argCount = findArguments(function, variableMap);
	int nextVariableAddress = -8;

	Lexeme* identifier = findFirstLexemeOccurence(function, LEX_IDENTIFIER);
	Lexeme* stmtlist = findFirstLexemeOccurence(function, LEX_STMTLIST);
	findAllVariables(&nextVariableAddress, stmtlist, variableMap);

	printf("next variable address value: %d\n", nextVariableAddress);
	fprintf(file, "# Function: %s%s (%d args)\n", FUNCTION_PREPEND, identifier->token->data, argCount);
	fprintf(file, "%s%s:\n", FUNCTION_PREPEND, identifier->token->data);

	compileStmtlist(file, stmtlist, variableMap);

	fprintf(file, "\n\n");
}

void compileToASM(FILE* file, Lexeme* head){
	Hashmap* functionsMap = createHashmap();

	Lexeme* child = head->firstChild;
	while(child != NULL){
		// Processes child iff it has a child underneath it
		// If there is nothing underneath it, then there isn't 
		// any code, ie it's an empty line.
		if(child->firstChild != NULL){
			compileFunction(file, child->firstChild, functionsMap);
		}

		// Move to next child
		child = child->nextSibling;
	}

	destroyHashmap(functionsMap);
}

