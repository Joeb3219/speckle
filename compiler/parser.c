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

Token** current = NULL;

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

void parseStmtList(Lexeme* head){
	Lexeme* stmtList = createLexeme();
	addChild(head, stmtList);

	if(isTokenType(END)) return;
	parseStmt(stmtList);
	if(!isTokenType(SEMICOLON)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), SEMICOLON);
	consume();

	parseStmtList(stmtList);
}

//<stmt>	:= <func> | <declaration> | <expression> | <while> | <return> | <if>
void parseStmt(Lexeme* head){
	Lexeme* stmt = createLexeme();
	addChild(head, stmt);

	if(isTokenType(WHILE)) parseWhile(stmt);
	else if(isTokenType(IF)) parseIf(stmt);
	else if(isTokenType(RET)) parseReturn(stmt);
	else if(isTokenType(FN)) parseFunc(stmt);
	else if(isTokenType(VAR)) parseDeclaration(stmt);
	else parseExpression(stmt);
}

void parseDeclaration(Lexeme* head){
	Lexeme* declaration = createLexeme();
	addChild(head, declaration);

	if(!isTokenType(VAR)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), VAR);
	consume();
	parseAssign(declaration);
}

void parseAssign(Lexeme* head){

}

void parseFunc(Lexeme* head){

}


void parseReturn(Lexeme* head){

}

void parseFuncCall(Lexeme* head){

}

void parseExpression(Lexeme* head){

}

void parseLogic(Lexeme* head){

}

void parseLeq(Lexeme* head){

}

void parseEquals(Lexeme* head){

}

void parseOr(Lexeme* head){

}

void parseAnd(Lexeme* head){

}

void parseNot(Lexeme* head){

}

void parseArgList(Lexeme* head){

}

void parseParamList(Lexeme* head){

}

void parseIf(Lexeme* head){

}

void parseWhile(Lexeme* head){

}

void parseSub(Lexeme* head){

}


void parseProgram(Lexeme* head){
	if(isTokenType(END)) ERR_UNEXPECTED_EOF(currentToken());
	if(isTokenType(UNKNOWN)) ERR_UNEXPECTED_TOKEN(currentToken());
	parseStmtList(head);

	if(isTokenType(END)) return;
	
	ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), END);
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
	current = &headToken;
	parseProgram(head);
	return head;
}