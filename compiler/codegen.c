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
void compileFunction(FILE* file, Lexeme* function, Hashmap* functionsMap, int* ifCounter);
int findArguments(Lexeme* function, Hashmap* hashmap);
Lexeme* findFirstLexemeOccurence(Lexeme* head, LexemeType type);
void compileStmtlist(FILE* file, Lexeme* stmtlist, Hashmap* variables, int* ifCounter);
void compileIf(FILE* file, Lexeme* ifNode, Hashmap* variables, int* ifCounter);
void compileWhile(FILE* file, Lexeme* whileNode, Hashmap* variables, int* ifCounter);
void compileMath(FILE* file, Lexeme* sub, Hashmap* variables, int* ifCounter);
void compileExpression(FILE* file, Lexeme* expression, Hashmap* variables, int* ifCounter);
void compileDeclaration(FILE* file, Lexeme* declaration, Hashmap* variables, int* ifCounter);
void compileAssign(FILE* file, Lexeme* assign, Hashmap* variables, int* ifCounter);
void compileStmt(FILE* file, Lexeme* stmt, Hashmap* variables, int* ifCounter);
void compileReturn(FILE* file, Lexeme* ret, Hashmap* variables, int* ifCounter);
void compileFuncCall(FILE* file, Lexeme* call, Hashmap* variables, int* ifCounter);
void compileLogic(FILE* file, Lexeme* logic, Hashmap* variables, int* ifCounter);
void compileIdentifierOrNumber(FILE* file, Lexeme* logic, Hashmap* variables, int* ifCounter);

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

	int currentPosition = 16;
	
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
		if(single->type == LEX_IDENTIFIER){
			// We have found an identifier! Call the press.
			hashmapInsert(variables, single->token->data, *current);
			(*current) -= INT_SIZE;
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
	if(ifNode == NULL || ifNode->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#if\n");

	// We generate two bodies: one for the jump, and one for not if the condition fails.

	Lexeme* expression = findFirstLexemeOccurence(ifNode, LEX_EXPRESSION);
	Lexeme* stmtlist = findFirstLexemeOccurence(ifNode, LEX_STMTLIST);

	int currentIfCount = *ifCounter;
	(*ifCounter) = currentIfCount + 1;

	// First, we compile the expression.
	// This will store some value in %rax
	// If this value is equal to 1, we will do a jump to if_branch_%d_success
	// Otherwise, we go to the fail branch.
	compileExpression(file, expression, variables, ifCounter);

	fprintf(file, "\tcmpq $1, %%rax\n");
	fprintf(file, "\tje .if_branch_%d_success\n", currentIfCount);
	// If we didn't jump previously, that means it's not equal, therefore we jump to the fail branch.
	fprintf(file, "\tjmp .if_branch_%d_fail\n", currentIfCount);

	// Now we start the success branch
	fprintf(file, ".if_branch_%d_success:\n", currentIfCount);

	// Now we can compile the statementlist
	compileStmtlist(file, stmtlist, variables, ifCounter);

	// Now we can compile the failure jump point
	fprintf(file, ".if_branch_%d_fail:\n", currentIfCount);

}

void compileWhile(FILE* file, Lexeme* whileNode, Hashmap* variables, int* ifCounter){
	if(whileNode == NULL || whileNode->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#while\n");

	// We generate two bodies: one for the jump, and one for not if the condition fails.

	Lexeme* expression = findFirstLexemeOccurence(whileNode, LEX_EXPRESSION);
	Lexeme* stmtlist = findFirstLexemeOccurence(whileNode, LEX_STMTLIST);

	int currentIfCount = *ifCounter;
	(*ifCounter) = currentIfCount + 1;

	// First, we leave the top of the loop structure.
	// This will be the boundary point we go to at the start of each iteration.
	fprintf(file, ".while_branch_%d_test:\n", currentIfCount);

	// Next, we compile the expression.
	// This will store some value in %rax
	// If this value is not equal to 1, we will continue.
	// If this value is ANYTHING OTHER THAN 1, we will skip past the statementlist body.
	compileExpression(file, expression, variables, ifCounter);

	fprintf(file, "\tcmpq $1, %%rax\n");
	fprintf(file, "\tjne .while_branch_%d_finish\n", currentIfCount);

	// If we didn't jump, then we are meant to execute the code.
	// Now we can compile the statementlist
	compileStmtlist(file, stmtlist, variables, ifCounter);

	// Now that we're done all of this, we jump back to the top of the while loop and see where we are at.
	fprintf(file, "\tjmp .while_branch_%d_test\n", currentIfCount);

	// Now we can compile the failure jump point
	fprintf(file, ".while_branch_%d_finish:\n", currentIfCount);

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
			compileIf(file, child, variables, ifCounter);
			break;
		case LEX_WHILE:
			compileWhile(file, child, variables, ifCounter);
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
		case LEX_MATH:
			compileMath(file, child, variables, ifCounter);
			break;
		case LEX_LOGIC:
			compileLogic(file, child, variables, ifCounter);
			break;
		case LEX_FUNCCALL:
			compileFuncCall(file, child, variables, ifCounter);
			break;
		case LEX_IDENTIFIER:
		case LEX_NUMBER:
		case LEX_ELEMENT:
			compileIdentifierOrNumber(file, child, variables, ifCounter);
			break;
		case LEX_ASSIGN:
			compileAssign(file, child, variables, ifCounter);
			break;
		case LEX_EXPRESSION:
			compileExpression(file, child, variables, ifCounter);
			break;
		default:
			printf("ERROR!!\n");
			break;
	}
}

void compileExpressionNonMath(FILE* file, Lexeme* expression, Hashmap* variables, int* ifCounter){
	if(expression == NULL || expression->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#expr_nonmath\n");
	int variableOffset;

	Lexeme* child = expression->firstChild;
	switch(child->type){
		case LEX_IDENTIFIER:
			variableOffset = hashmapRead(variables, child->token->data);
			if(variableOffset == HASHMAP_UNKNOWN_VALUE) ERR_UNKNOWN_IDENTIFIER(child->token);
			fprintf(file, "\tmovq %d(%%rbp), %%rax\n", variableOffset);
			break;
		case LEX_NUMBER:
			fprintf(file, "\tmovq $%d, %%rax\n", atoi(child->token->data));
			break;
		case LEX_LOGIC:
			compileLogic(file, child, variables, ifCounter);
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

void compileLogic(FILE* file, Lexeme* logic, Hashmap* variables, int* ifCounter){
	if(logic == NULL || logic->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#logic\n");
	
	Lexeme* child = logic->firstChild;
	Lexeme *left, *right;

	switch(child->type){
		case LEX_LEQ:
			left = child->firstChild;
			right = left->nextSibling;

			// First, we parse the left side and its result will be stored in %rax, to be moved to %rcx
			compileIdentifierOrNumber(file, left, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rcx\n");
			// Next, we parse the left side and its result will be stored in %rax, to be moved to %rdx
			compileIdentifierOrNumber(file, right, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rdx\n");

			// Now we evaluate the leq status, and then store it into %rax
			fprintf(file, "\tcmpq %%rdx, %%rcx\n");
			fprintf(file, "\tsetle %%al\n");
			fprintf(file, "\tmovzbq %%al, %%rax\n");
			break;
		case LEX_GEQ:
			left = child->firstChild;
			right = left->nextSibling;

			// First, we parse the left side and its result will be stored in %rax, to be moved to %rcx
			compileIdentifierOrNumber(file, left, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rcx\n");
			// Next, we parse the left side and its result will be stored in %rax, to be moved to %rdx
			compileIdentifierOrNumber(file, right, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rdx\n");

			// Now we evaluate the leq status, and then store it into %rax
			fprintf(file, "\tcmpq %%rdx, %%rcx\n");
			fprintf(file, "\tsetge %%al\n");
			fprintf(file, "\tmovzbq %%al, %%rax\n");
			break;
		case LEX_GREATER:
			left = child->firstChild;
			right = left->nextSibling;

			// First, we parse the left side and its result will be stored in %rax, to be moved to %rcx
			compileIdentifierOrNumber(file, left, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rcx\n");
			// Next, we parse the left side and its result will be stored in %rax, to be moved to %rdx
			compileIdentifierOrNumber(file, right, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rdx\n");

			// Now we evaluate the leq status, and then store it into %rax
			fprintf(file, "\tcmpq %%rdx, %%rcx\n");
			fprintf(file, "\tsetg %%al\n");
			fprintf(file, "\tmovzbq %%al, %%rax\n");
			break;
		case LEX_LESS:
			left = child->firstChild;
			right = left->nextSibling;

			// First, we parse the left side and its result will be stored in %rax, to be moved to %rcx
			compileIdentifierOrNumber(file, left, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rcx\n");
			// Next, we parse the left side and its result will be stored in %rax, to be moved to %rdx
			compileIdentifierOrNumber(file, right, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rdx\n");

			// Now we evaluate the leq status, and then store it into %rax
			fprintf(file, "\tcmpq %%rdx, %%rcx\n");
			fprintf(file, "\tsetl %%al\n");
			fprintf(file, "\tmovzbq %%al, %%rax\n");
			break;
		case LEX_EQUALS:
			left = child->firstChild;
			right = left->nextSibling;

			// First, we parse the left side and its result will be stored in %rax, to be moved to %rcx
			compileIdentifierOrNumber(file, left, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rcx\n");
			// Next, we parse the left side and its result will be stored in %rax, to be moved to %rdx
			compileIdentifierOrNumber(file, right, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rdx\n");

			// Now we evaluate the leq status, and then store it into %rax
			fprintf(file, "\tcmpq %%rcx, %%rdx\n");
			fprintf(file, "\tsetz %%al\n");
			fprintf(file, "\tmovzbq %%al, %%rax\n");
			break;
		case LEX_AND:
			left = child->firstChild;
			right = left->nextSibling;

			// First, we parse the left side and its result will be stored in %rax, to be moved to %rcx
			compileIdentifierOrNumber(file, left, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rcx\n");
			// Next, we parse the left side and its result will be stored in %rax, to be moved to %rdx
			compileIdentifierOrNumber(file, right, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rdx\n");

			// Now we evaluate the leq status, and then store it into %rax
			fprintf(file, "\tandq %%rcx, %%rdx\n");
			fprintf(file, "\tmovq %%rdx, %%rax\n");
			break;
		case LEX_OR:
			left = child->firstChild;
			right = left->nextSibling;

			// First, we parse the left side and its result will be stored in %rax, to be moved to %rcx
			compileIdentifierOrNumber(file, left, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rcx\n");
			// Next, we parse the left side and its result will be stored in %rax, to be moved to %rdx
			compileIdentifierOrNumber(file, right, variables, ifCounter);
			fprintf(file, "\tmovq %%rax, %%rdx\n");

			// Now we evaluate the leq status, and then store it into %rax
			fprintf(file, "\torq %%rcx, %%rdx\n");
			fprintf(file, "\tmovq %%rdx, %%rax\n");
			break;
		case LEX_NOT:
			left = child->firstChild;
			
			// First, we parse the left side and its result will be stored in %rax, to be moved to %rcx
			compileIdentifierOrNumber(file, left, variables, ifCounter);

			fprintf(file, "\ttest %%rax, %%rax\n");
			fprintf(file, "\tsetz %%al\n");
			fprintf(file, "\tmovzbq %%al, %%rax\n");
			break;
		default:
			printf("ERROR!!!!\n");
			break;
	}
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

void compileIdentifierOrNumber(FILE* file, Lexeme* node, Hashmap* variables, int* ifCounter){
	if(node == NULL) return;
	int variableOffset;

	switch(node->type){
		case LEX_IDENTIFIER:
			variableOffset = hashmapRead(variables, node->token->data);
			if(variableOffset == HASHMAP_UNKNOWN_VALUE) ERR_UNKNOWN_IDENTIFIER(node->token);
			fprintf(file, "\tmovq %d(%%rbp), %%rax\n", variableOffset);
			break;
		case LEX_NUMBER:
			fprintf(file, "\tmovq $%d, %%rax\n", atoi(node->token->data));
			break;
		case LEX_ELEMENT:
			// We store the base variable address in %rcx
			variableOffset = hashmapRead(variables, node->firstChild->token->data);
			if(variableOffset == HASHMAP_UNKNOWN_VALUE) ERR_UNKNOWN_IDENTIFIER(node->token);
			fprintf(file, "\tmovq %d(%%rbp), %%rcx\n", variableOffset);
			compileIdentifierOrNumber(file, node->firstChild->nextSibling, variables, ifCounter);

			// Normally, we'd use LEA, but afaik we can't do that with two registers.
			// Instead, we add the offset now stored in %rax to the base variable stored in %rcx.
			fprintf(file, "\timulq $8, %%rax\n");
			fprintf(file, "\taddq %%rax, %rcx\n");

			// We now need to move the contents of the (%rcx) to the return register -- %rax
			fprintf(file, "\tmovq 0(%%rcx), %%rax\n");
			break;
		default:
			printf("ERROR!!!!!!\n");
			break;
	}
}

void compileMath(FILE* file, Lexeme* math, Hashmap* variables, int* ifCounter){
	if(math == NULL || math->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#math\n");
	
	// Expression will, by default, store everything in %rax
	// Thus, we will perform the left computation and then move %rax to %rbx.
	compileIdentifierOrNumber(file, math->firstChild->firstChild, variables, ifCounter);
	fprintf(file, "\tmovq %%rax, %%rbx\n");

	// Then, we can go ahead and compute the left side, stored in %rax, and then subtract %rbx - %rax
	compileIdentifierOrNumber(file, math->firstChild->firstChild->nextSibling, variables, ifCounter);

	// We now will have a in %rbx, and b in %rax
	// We will perform whatever computation is needed and store it in %rbx
	switch(math->firstChild->type){
		case LEX_SUB:
			fprintf(file, "\tsubq %%rax, %%rbx\n");
			fprintf(file, "\tmovq %%rbx, %%rax\n");
			break;
		case LEX_ADD:
			fprintf(file, "\taddq %%rbx, %%rax\n");
			break;
		case LEX_MULT:
			fprintf(file, "\timulq %%rbx, %%rax\n");
			break;
		case LEX_DIV:
			// The assembly divide instruction takes the given register, divides %rax by it, and then places the result
			// into %rax
			// Thus, we have to do some movement to make this do the correct computation.
			fprintf(file, "\tmovq %%rax, %%rcx\n");
			fprintf(file, "\tmovq %%rbx, %%rax\n");
			fprintf(file, "\tcqto\n");
			fprintf(file, "\tidivq %%rcx\n");
			break;
		case LEX_MOD:
			// The assembly divide instruction takes the given register, divides %rax by it, and then places the result
			// into %rax
			// Thus, we have to do some movement to make this do the correct computation.
			fprintf(file, "\tmovq %%rax, %%rcx\n");
			fprintf(file, "\tmovq %%rbx, %%rax\n");
			fprintf(file, "\tcqto\n");
			fprintf(file, "\tidivq %%rcx\n");
			fprintf(file, "\tmovq %%rdx, %%rax\n");
			break;
		default:
			printf("Uh oh, unknown math\n");
			break;
	}
}

void compileDeclaration(FILE* file, Lexeme* declaration, Hashmap* variables, int* ifCounter){
	if(declaration == NULL || declaration->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#declaration\n");

	Lexeme* identifier = declaration->firstChild;
	Lexeme* right = identifier->nextSibling;
	int variableOffset = hashmapRead(variables, identifier->token->data);
	if(variableOffset == HASHMAP_UNKNOWN_VALUE) ERR_UNKNOWN_IDENTIFIER(identifier->token);

	// If there is no right side, then the user simply declared but didn't instantiate.
	// We will set it to 0 for them.
	if(right == NULL){
		fprintf(file, "\tmovq $0, %d(%%rbp)\n", variableOffset);
		return;
	}

	// If the right side is an array declaration, then we need to create that memory on the heap.
	if(right->type == LEX_ARRAY){
		// The number of elements will strictly be either a number or an identifier, so we
		// go ahead and load it into %rax.
		compileIdentifierOrNumber(file, right->firstChild, variables, ifCounter);
		fprintf(file, "\tpushq %%rax\n");		
		fprintf(file, "\tcall %s%s\n", FUNCTION_PREPEND, "malloc");
	}else compileExpression(file, right, variables, ifCounter);

	// Regardless, a value is pushed to %%rax, so we can just move it on into the right area.
	fprintf(file, "\tmovq %%rax, %d(%rbp)\n", variableOffset);
}

void compileAssign(FILE* file, Lexeme* assign, Hashmap* variables, int* ifCounter){
	if(assign == NULL || assign->firstChild == NULL) return;
	if(PRINT_LABELS_IN_ASM) fprintf(file, "\t#assign\n");

	Lexeme* left = assign->firstChild;
	Lexeme* right = left->nextSibling;
	int variableOffset;

	// If the right side is an array declaration, then we need to create that memory on the heap.
	if(right->type == LEX_ARRAY){
		// The number of elements will strictly be either a number or an identifier, so we
		// go ahead and load it into %rax.
		compileIdentifierOrNumber(file, right->firstChild, variables, ifCounter);
		fprintf(file, "\tpushq %%rax\n");		
		fprintf(file, "\tcall %s%s\n", FUNCTION_PREPEND, "malloc");
	}else compileExpression(file, right, variables, ifCounter);

	// Now, we focus on what we can do with this left side.
	// Regardless of what it is, we will have a variableOffset for the identifier part of it (elements have an identifier attached).
	if(left->type == LEX_ELEMENT){
		// We store the base variable address in %rcx
		// We then move the expression's answer to %rdx, as evaluating the offset will take place in %rax.
		variableOffset = hashmapRead(variables, left->firstChild->token->data);
		if(variableOffset == HASHMAP_UNKNOWN_VALUE) ERR_UNKNOWN_IDENTIFIER(left->firstChild->token);
		fprintf(file, "\tmovq %d(%%rbp), %%rcx\n", variableOffset);
		fprintf(file, "\tmovq %%rax, %%rdx\n");
		compileIdentifierOrNumber(file, left->firstChild->nextSibling, variables, ifCounter);

		// Normally, we'd use LEA, but afaik we can't do that with two registers.
		// Instead, we add the offset now stored in %rax to the base variable stored in %rcx.
		fprintf(file, "\timulq $8, %%rax\n");
		fprintf(file, "\taddq %%rax, %rcx\n");

		// Now, our resultant answer is stored in %rdx, and the destination address is in %rcx.
		fprintf(file, "\tmovq %%rdx, 0(%%rcx)\n");
	}else{
		variableOffset = hashmapRead(variables, left->token->data);
		if(variableOffset == HASHMAP_UNKNOWN_VALUE) ERR_UNKNOWN_IDENTIFIER(left->token);
		fprintf(file, "\tmovq %%rax, %d(%%rbp)\n", variableOffset);
	}
}

void compileFunction(FILE* file, Lexeme* function, Hashmap* functionsMap, int *ifCounter){
	// Ensure that we are dealing with a function declaration
	if(function->type != LEX_FUNC) ERR_UNEXPECTED_LEXEME_EXPECTED(function, LEX_FUNC);
	Hashmap* variableMap = createHashmap();

	int argCount = findArguments(function, variableMap);
	int nextVariableAddress = -8;

	Lexeme* identifier = findFirstLexemeOccurence(function, LEX_IDENTIFIER);
	Lexeme* stmtlist = findFirstLexemeOccurence(function, LEX_STMTLIST);
	findAllVariables(&nextVariableAddress, stmtlist, variableMap);

	fprintf(file, "# Function: %s%s (%d args)\n", FUNCTION_PREPEND, identifier->token->data, argCount);
	fprintf(file, ".globl %s%s\n", FUNCTION_PREPEND, identifier->token->data);
	fprintf(file, ".type %s%s, @function\n", FUNCTION_PREPEND, identifier->token->data);
	fprintf(file, "%s%s:\n", FUNCTION_PREPEND, identifier->token->data);

	fprintf(file, ".body_fn_%s:\n", identifier->token->data);
	fprintf(file, "\tpushq %%rbp\n");
	fprintf(file, "\tmovq %%rsp, %%rbp\n");
	fprintf(file, "\tsubq $%d, %%rsp\n", -nextVariableAddress);

	compileStmtlist(file, stmtlist, variableMap, ifCounter);

	// We explicitly add a return instruction incase the user forgets to include one,
	// which will an error value of -1.
	fprintf(file, "\tmovq $-1, %%rax\n");
	fprintf(file, "\tmovq %%rbp, %%rsp\n");
	fprintf(file, "\tpopq %%rbp\n");
	fprintf(file, "\tret\n");

	fprintf(file, "\n\n");

	destroyHashmap(variableMap);
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

    int ifCounter = 0;

	Lexeme* child = head->firstChild;
	while(child != NULL){
		// Processes child iff it has a child underneath it
		// If there is nothing underneath it, then there isn't 
		// any code, ie it's an empty line.
		if(child->firstChild != NULL){
			compileFunction(file, child->firstChild, functionsMap, &ifCounter);
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

	// A function to read one byte in from the stdin.
	fprintf(file, ".globl %s%s\n", FUNCTION_PREPEND, "read");
    fprintf(file, ".type %s%s, @function\n", FUNCTION_PREPEND, "read");
	fprintf(file, "%s%s:\n", FUNCTION_PREPEND, "read");
	fprintf(file, "\tpushq %%rbp\n");
	fprintf(file, "\tmovq %%rsp, %%rbp\n");
	fprintf(file, "\tsubq $8, %%rsp\n");
  	fprintf(file, "\tmovq stdin(%rip), %rax\n");
  	fprintf(file, "\tmovq %rax, %%rdi\n");
  	fprintf(file, "\tmovq $0, %rax\n");
  	fprintf(file, "\tcall getchar\n");
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

	// A malloc function
	fprintf(file, ".globl %s%s\n", FUNCTION_PREPEND, "malloc");
	fprintf(file, ".type %s%s, @function\n", FUNCTION_PREPEND, "malloc");
	fprintf(file, "%s%s:\n", FUNCTION_PREPEND, "malloc");
	fprintf(file, "\tpushq %%rbp\n");
	fprintf(file, "\tmovq %%rsp, %%rbp\n");
	fprintf(file, "\tsubq $16, %%rsp\n");
  	fprintf(file, "\tmovq 16(%%rbp), %%rax\n");
  	fprintf(file, "\taddq $1, %%rax\n");
	fprintf(file, "\tsalq $3, %rax\n");
    fprintf(file, "\tcltq\n");
    fprintf(file, "\tmovq %%rax, %%rdi\n");
  	fprintf(file, "\tmovl $0, %%eax\n");
  	fprintf(file, "\tcall malloc\n");
  	fprintf(file, "\tmovq 16(%%rbp), %%rbx\n");
  	fprintf(file, "\tmovq %%rbx, 0(%%rax)\n");
  	fprintf(file, "\taddq $8, %%rax\n");
	fprintf(file, "\tmovq %%rbp, %%rsp\n");
	fprintf(file, "\tpopq %%rbp\n");
	fprintf(file, "\tret\n");

	// A malloc function
	fprintf(file, ".globl %s%s\n", FUNCTION_PREPEND, "len");
	fprintf(file, ".type %s%s, @function\n", FUNCTION_PREPEND, "len");
	fprintf(file, "%s%s:\n", FUNCTION_PREPEND, "len");
	fprintf(file, "\tpushq %%rbp\n");
	fprintf(file, "\tmovq %%rsp, %%rbp\n");
	fprintf(file, "\tsubq $16, %%rsp\n");
  	fprintf(file, "\tmovq 16(%%rbp), %%rax\n");
  	fprintf(file, "\tleaq -8(%%rax), %%rax\n");
  	fprintf(file, "\tmovq 0(%%rax), %rax\n");
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

