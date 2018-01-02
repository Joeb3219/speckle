#ifndef _CODEGEN_H_
	#define _CODEGEN_H_
	
	#include "parser.h"
	#include "hashmap.h"

	// Possible instructions:
	// 	Function declaration
	// 	If statement
	// 	Expression -- math, logical
	// 	Function call
	// 	While loop
	

	enum InstructionType{
		FUNCTION_DECLARATION, IF_STATEMENT, EXPRESSION_STATEMENT, FUNCTION_CALL, WHILE_LOOP
	};

	struct FunctionDeclaration{
		
	};

	typedef struct IfStatement IfStatement;



	typedef enum InstructionType InstructionType;

	struct Instruction{

	};

	struct Function{
		char* name;
		Lexeme* lexemes;
		Hashmap* variableMap;
	};

	typedef struct Function Function;

	struct Program{
		Function** functions;
		int numFunctions;
	};

	typedef struct Program Program;

	Program* createProgram();
	void destroyProgram(Program* program);
	void addFunctionToProgram(Program* program, Function* function);
	Function* createFunction();
	void destroyFunction(Function* function);
	
	Program* lexemesToProgram(Lexeme* head);
	void programToC(Program* program);


#endif