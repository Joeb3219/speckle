#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"
#include "speckle.h"
#include "error.h"

#define currentToken()	((*current))
#define consume()		(current = &(*current)->next)
#define isTokenType(T)	(currentToken()->type == T)

void addChild(Lexeme* parent, Lexeme* child){
	if(child == NULL || parent == NULL) return;
	child->parent = parent;

	Lexeme* lastSibling = parent->firstChild;
	if(lastSibling == NULL) parent->firstChild = child;
	else{
		while(lastSibling->nextSibling != NULL) lastSibling = lastSibling->nextSibling;
		lastSibling->nextSibling = child;
	}
}

int parseStmtList(Lexeme* head, Token** current){

}

int parseStmt(Lexeme* head, Token** current){

}

int parseDeclaration(Lexeme* head, Token** current){

}

int parseAssign(Lexeme* head, Token** current){

}

int parseFunc(Lexeme* head, Token** current){

}

int parseFuncCall(Lexeme* head, Token** current){

}

int parseExpression(Lexeme* head, Token** current){

}

int parseLogic(Lexeme* head, Token** current){

}

int parseLeq(Lexeme* head, Token** current){

}

int parseEquals(Lexeme* head, Token** current){

}

int parseOr(Lexeme* head, Token** current){

}

int parseAnd(Lexeme* head, Token** current){

}

int parseNot(Lexeme* head, Token** current){

}

int parseArgList(Lexeme* head, Token** current){

}

int parseParamList(Lexeme* head, Token** current){

}

int parseIf(Lexeme* head, Token** current){

}

int parseWhile(Lexeme* head, Token** current){

}

int parseSub(Lexeme* head, Token** current){

}


int parseProgram(Lexeme* head, Token** current){
	if(isTokenType(END)) ERR_UNEXPECTED_EOF(currentToken());
	if(isTokenType(UNKNOWN)) ERR_UNEXPECTED_TOKEN(currentToken());
	int status = parseStmtList(head, current);

	if(isTokenType(END)) return 1;
	
	ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), END);
	return 0;
}

Lexeme* createLexeme(){
	Lexeme* lexeme = malloc(sizeof(Lexeme));
	lexeme->parent = lexeme->nextSibling = lexeme->prevSibling = lexeme->firstChild = NULL;
	lexeme->token = NULL;
	return lexeme;
}

void destroyLexeme(Lexeme* lexeme){
	free(lexeme);
}

void destroyTree(Lexeme* head){
	// First, we go as far down as we can
	if(head->firstChild != NULL) destroyTree(head->firstChild);

	// Once we've gotten all the way to the bottom, we attempt to move as far right as possible.
	if(head->nextSibling != NULL) destroyTree(head->nextSibling);

	// And now we can go ahead and destroy the node we are looking at, since we are the furthest right bottom most node.
	destroyLexeme(head);
}

Lexeme* parse(Arguments* args, Token* headToken){
	Lexeme* head = createLexeme();
	int status = parseProgram(head, &headToken);
	return head;
}