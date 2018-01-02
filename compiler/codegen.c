#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "speckle.h"
#include "error.h"
#include "parser.h"
#include "lexer.h"
#include "hashmap.h"

Program* createProgram(){
	Program* program = malloc(sizeof(Program));
	program->functions = NULL;
	program->numFunctions = 0;
	return program;
}

void destroyProgram(Program* program){
	int i = 0;
	if(program->functions != NULL){
		for(i = 0; i < program->numFunctions; i ++){
			destroyFunction(program->functions[i]);
		}
		free(program->functions);
	}

	free(program);
}

Function* createFunction(){
	Function* function = malloc(sizeof(Function));
	function->name = NULL;
	function->lexemes = NULL;
	function->variableMap = createHashmap();
	return function;
}

void destroyFunction(Function* function){
	if(function->lexemes != NULL) free(function->lexemes);
	destroyHashmap(function->variableMap);
	free(function);
}

void addFunctionToProgram(Program* program, Function* function){
	if(program->numFunctions == 0) program->functions = malloc(sizeof(Function) * 1);
	else{
		Function** functions = realloc(program->functions, sizeof(Function) * (program->numFunctions + 1));
		if(functions == NULL) error_log(SEVERE, "Allocation failure.");
		program->functions = functions;
	}
	program->functions[program->numFunctions] = function;
	program->numFunctions ++;
}



Program* lexemesToProgram(Lexeme* head){
	Program* program = createProgram();

	Function* main = createFunction();
	main->name = "main";
	addFunctionToProgram(program, main);

	Function* function;


	return program;
}

void programToC(Program* program){

}

