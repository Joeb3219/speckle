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
#define PRINT_LABELS_IN_ASM 0

// Private function declarations
void compileFunction(FILE* file, Lexeme* function, Hashmap* functionsMap);
int findArguments(Lexeme* function, Hashmap* hashmap);
Lexeme* findFirstLexemeOccurence(Lexeme* head, LexemeType type);
void compileStmtlist(FILE* file, Lexeme* stmtlist, Hashmap* variables, int* ifCounter);
void compileIf(FILE* file, Lexeme* ifNode, Hashmap* variables, int* ifCounter);
void compileSub(FILE* file, Lexeme* sub, Hashmap* variables, int* ifCounter);
void compileExpression(FILE* file, Lexeme* expression, Hashmap* variables, int* ifCounter);
void compileExpressionNonMath(FILE* file, Lexeme* expression, Hashmap* variables, int* ifCounter);
void compileDeclaration(FILE* file, Lexeme* declaration, Hashmap* variables, int* ifCounter);
void compileAssign(FILE* file, Lexeme* assign, Hashmap* variables, int* ifCounter);
void compileStmt(FILE* file, Lexeme* stmt, Hashmap* variables, int* ifCounter);
void compileReturn(FILE* file, Lexeme* ret, Hashmap* variables, int* ifCounter);
void compileFuncCall(FILE* file, Lexeme* call, Hashmap* variables, int* ifCounter);

// Function implementations

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

void compileStmtlist(FILE* file, Lexeme* stmtlist, Hashmap* variables, int* ifCounter){
	if(stmtlist == NULL) return;
	if(stmtlist->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#stmtlist\n");

	compileStmt(file, stmtlist->firstChild, variables, ifCounter);

	if(stmtlist->firstChild->nextSibling == NULL) return;
	compileStmtlist(file, stmtlist->firstChild->nextSibling, variables, ifCounter);
}

void compileIf(FILE* file, Lexeme* ifNode, Hashmap* variables, int* ifCounter){

}

void compileStmt(FILE* file, Lexeme* stmt, Hashmap* variables, int* ifCounter){
	if(stmt == NULL || stmt->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#stmt\n");

	Lexeme* child = stmt->firstChild;
	switch(child->type){
		case LEX_DECLARATION:
			compileDeclaration(file, child, variables, ifCounter);
			break;
		case LEX_EXPRESSION:
			compileExpression(file, child, variables, ifCounter);
			break;
		case LEX_RETURN:
			compileReturn(file, child, variables, ifCounter);
			break;
		case LEX_IF:
			break;
		default:
			printf("ERROR!!\n");
			break;
	}
}

void compileExpression(FILE* file, Lexeme* expression, Hashmap* variables, int* ifCounter){
	if(expression == NULL || expression->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#expression\n");

	Lexeme* child = expression->firstChild;
	switch(child->type){
		case LEX_SUB:
			compileSub(file, child, variables, ifCounter);
			break;
		case LEX_EXPRESSION_NONMATH:
			compileExpressionNonMath(file, child, variables, ifCounter);
			break;
		default:
			printf("ERROR!!\n");
			break;
	}
}

void compileExpressionNonMath(FILE* file, Lexeme* expression, Hashmap* variables, int* ifCounter){
	if(expression == NULL || expression->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#expr_nonmath\n");

	Lexeme* child = expression->firstChild;
	switch(child->type){
		case LEX_IDENTIFIER:
			fprintf(file, "\tmovq %d(%%rbp), %%rax\n", hashmapRead(variables, child->token->data));
			break;
		case LEX_NUMBER:
			fprintf(file, "\tmovq $%d, %%rax\n", atoi(child->token->data));
			break;
		case LEX_LOGIC:
			break;
		case LEX_ASSIGN:
			compileAssign(file, child, variables, ifCounter);
			break;
		case LEX_FUNCCALL:
			compileFuncCall(file, child, variables, ifCounter);
			break;
		default:
			printf("====\n");
			printf("\nERROR\n");
			break;
	}
}

void compileReturn(FILE* file, Lexeme* ret, Hashmap* variables, int* ifCounter){
	if(ret == NULL || ret->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#return\n");
	// First, we compile down the expression contained, which will output to %rax.
	// We can then return directly from here.

	compileExpression(file, ret->firstChild, variables, ifCounter);

	fprintf(file, "\tmovq %%rbp, %%rsp\n");
	fprintf(file, "\tpopq %%rbp\n");
	fprintf(file, "\tret\n");
}

void compileFuncCall(FILE* file, Lexeme* call, Hashmap* variables, int* ifCounter){
	if(call == NULL || call->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#funccall\n");

	Lexeme* identifier = findFirstLexemeOccurence(call, LEX_IDENTIFIER);
	Lexeme* argsList = findFirstLexemeOccurence(call, LEX_ARGLIST);

	Lexeme* child = argsList->firstChild;
	Lexeme* originalChild = argsList->firstChild;
	if(child != NULL){
		// We first must go from the very last argument and begin pushing them from right to left.
		while(child->nextSibling != NULL){
			child = child->nextSibling->firstChild;
		}

		// We now have child as the very last sibling in the list.
		// For each child, we process the expression, which stores a value into %rax, and then push %rax.
		while(child->parent->type == LEX_ARGLIST){
			compileExpression(file, child, variables, ifCounter);
			fprintf(file, "\tpushq %rax\n");
			child = child->parent->prevSibling;
		}
	}

	fprintf(file, "\tcall %s%s\n", FUNCTION_PREPEND, identifier->token->data);
}

void compileSub(FILE* file, Lexeme* sub, Hashmap* variables, int* ifCounter){
	if(sub == NULL || sub->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#sub\n");
	
	// Expression will, by default, store everything in %rax
	// Thus, we will perform the left computation and then move %rax to %rbx.
	compileExpressionNonMath(file, sub->firstChild, variables, ifCounter);
	fprintf(file, "\tmovq %%rax, %%rbx\n");

	// Then, we can go ahead and compute the left side, stored in %rax, and then subtract %rbx - %rax
	compileExpressionNonMath(file, sub->firstChild->nextSibling, variables, ifCounter);

	fprintf(file, "\tsubq %%rbx, %%rax\n");
	fprintf(file, "\timulq $-1, %%rax\n");
}

void compileDeclaration(FILE* file, Lexeme* declaration, Hashmap* variables, int* ifCounter){
	if(declaration == NULL || declaration->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#declaration\n");
	compileAssign(file, declaration->firstChild, variables, ifCounter);
}

void compileAssign(FILE* file, Lexeme* assign, Hashmap* variables, int* ifCounter){
	if(assign == NULL || assign->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#assign\n");

	Lexeme* identifier = findFirstLexemeOccurence(assign, LEX_IDENTIFIER);
	Lexeme* expression = findFirstLexemeOccurence(assign, LEX_EXPRESSION);

	// First, we must generate the expression code needed to create the result.
	// We will then store this in the register %rax

	compileExpression(file, expression, variables, ifCounter);

	int variableOffset = hashmapRead(variables, identifier->token->data);

	fprintf(file, "\tmovq %%rax, %d(%rbp)\n", variableOffset);
}

void compileFunction(FILE* file, Lexeme* function, Hashmap* functionsMap){
	// Ensure that we are dealing with a function declaration
	if(function->type != LEX_FUNC) ERR_UNEXPECTED_LEXEME_EXPECTED(function, LEX_FUNC);
	Hashmap* variableMap = createHashmap();

	int argCount = findArguments(function, variableMap);
	int nextVariableAddress = -8;
	int intMarker = 0;

	Lexeme* identifier = findFirstLexemeOccurence(function, LEX_IDENTIFIER);
	Lexeme* stmtlist = findFirstLexemeOccurence(function, LEX_STMTLIST);
	findAllVariables(&nextVariableAddress, stmtlist, variableMap);

	fprintf(file, "# Function: %s%s (%d args)\n", FUNCTION_PREPEND, identifier->token->data, argCount);
	fprintf(file, ".globl %s%s\n", FUNCTION_PREPEND, identifier->token->data);
	fprintf(file, ".type %s%s, @function\n", FUNCTION_PREPEND, identifier->token->data);
	fprintf(file, "%s%s:\n", FUNCTION_PREPEND, identifier->token->data);

	fprintf(file, ".body:\n");
	fprintf(file, "\tpushq %%rbp\n");
	fprintf(file, "\tmovq %%rsp, %%rbp\n");
	fprintf(file, "\tsubq $%d, %%rsp\n", -nextVariableAddress);

	compileStmtlist(file, stmtlist, variableMap, &intMarker);

	fprintf(file, "\n\n");
}

void compileToASM(FILE* file, Lexeme* head){
	Hashmap* functionsMap = createHashmap();

	// Strings used in program execution
	fprintf(file, "printNumFormat:\n");
	fprintf(file, "\t.ascii \"%%d\\0\"\n");
    fprintf(file, "\t.text\n");
    fprintf(file, "newLineFormat:\n");
	fprintf(file, "\t.ascii \"\\n\\0\"\n");
    fprintf(file, "\t.text\n");
    fprintf(file, "printCharFormat:\n");
	fprintf(file, "\t.ascii \"%%c\\0\"\n");
    fprintf(file, "\t.text\n");

    fprintf(file, "\n\n");

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

	// A print function for printing a number
	fprintf(file, ".globl %s%s\n", FUNCTION_PREPEND, "printn");
    fprintf(file, ".type %s%s, @function\n", FUNCTION_PREPEND, "printn");
	fprintf(file, "%s%s:\n", FUNCTION_PREPEND, "printn");
	fprintf(file, "\tpushq %%rbp\n");
	fprintf(file, "\tmovq %%rsp, %%rbp\n");
	fprintf(file, "\tsubq $16, %%rsp\n");
  	fprintf(file, "\tmovq $printNumFormat, %%rdi\n");
  	fprintf(file, "\tmovq 16(%%rbp), %%rsi\n");
  	fprintf(file, "\tmovl $0, %%eax\n");
  	fprintf(file, "\tcall printf\n");
	fprintf(file, "\tmovq %%rbp, %%rsp\n");
	fprintf(file, "\tpopq %%rbp\n");
	fprintf(file, "\tret\n");

	// A print function for printing a character
	fprintf(file, ".globl %s%s\n", FUNCTION_PREPEND, "printc");
    fprintf(file, ".type %s%s, @function\n", FUNCTION_PREPEND, "printc");
	fprintf(file, "%s%s:\n", FUNCTION_PREPEND, "printc");
	fprintf(file, "\tpushq %%rbp\n");
	fprintf(file, "\tmovq %%rsp, %%rbp\n");
	fprintf(file, "\tsubq $16, %%rsp\n");
  	fprintf(file, "\tmovq $printCharFormat, %%rdi\n");
  	fprintf(file, "\tmovq 16(%%rbp), %%rsi\n");
  	fprintf(file, "\tmovl $0, %%eax\n");
  	fprintf(file, "\tcall printf\n");
	fprintf(file, "\tmovq %%rbp, %%rsp\n");
	fprintf(file, "\tpopq %%rbp\n");
	fprintf(file, "\tret\n");

	// A print function for printing a newline
	fprintf(file, ".globl %s%s\n", FUNCTION_PREPEND, "newline");
    fprintf(file, ".type %s%s, @function\n", FUNCTION_PREPEND, "newline");
	fprintf(file, "%s%s:\n", FUNCTION_PREPEND, "newline");
	fprintf(file, "\tpushq %%rbp\n");
	fprintf(file, "\tmovq %%rsp, %%rbp\n");
	fprintf(file, "\tsubq $8, %%rsp\n");
  	fprintf(file, "\tmovq $newLineFormat, %%rdi\n");
  	fprintf(file, "\tmovl $0, %%eax\n");
  	fprintf(file, "\tcall printf\n");
	fprintf(file, "\tmovq %%rbp, %%rsp\n");
	fprintf(file, "\tpopq %%rbp\n");
	fprintf(file, "\tret\n");

	// A main function
	fprintf(file, ".globl main\n");
    fprintf(file, ".type main, @function\n");
	fprintf(file, "main:\n");
	fprintf(file, "\tpushq %%rbp\n");
	fprintf(file, "\tmovq %%rsp, %%rbp\n");
	fprintf(file, "\tsubq $8, %%rsp\n");
	fprintf(file, "\tcall %s%s\n", FUNCTION_PREPEND, "main");

	fprintf(file, "\tleave\n");
	fprintf(file, "\tret\n");

	destroyHashmap(functionsMap);
}

