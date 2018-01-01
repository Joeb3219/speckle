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
#define peek()			((currentToken() == NULL) ? NULL : currentToken()->next)
#define isPeekType(T)	(peek() != NULL && peek()->type == T)

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
void parseIdentifier(Lexeme* head){
	Lexeme* identifier = createLexeme(LEX_IDENTIFIER);
	addChild(head, identifier);

	if(!isTokenType(IDENTIFIER)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), IDENTIFIER);
	consume();
}

void parseNumber(Lexeme* head){
	Lexeme* number = createLexeme(LEX_NUMBER);
	addChild(head, number);

	if(!isTokenType(NUMBER)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), NUMBER);
	consume();
}

void parseStmtList(Lexeme* head){
	Lexeme* stmtList = createLexeme(LEX_STMTLIST);
	addChild(head, stmtList);

	if(isTokenType(END)) return;
	parseStmt(stmtList);
	if(!isTokenType(SEMICOLON)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), SEMICOLON);
	consume();

	parseStmtList(stmtList);
}

//<stmt>	:= <func> | <declaration> | <expression> | <while> | <return> | <if>
void parseStmt(Lexeme* head){
	Lexeme* stmt = createLexeme(LEX_STMT);
	addChild(head, stmt);

	if(isTokenType(WHILE)) parseWhile(stmt);
	else if(isTokenType(IF)) parseIf(stmt);
	else if(isTokenType(RET)) parseReturn(stmt);
	else if(isTokenType(FN)) parseFunc(stmt);
	else if(isTokenType(VAR)) parseDeclaration(stmt);
	else parseExpression(stmt);
}

void parseDeclaration(Lexeme* head){
	Lexeme* declaration = createLexeme(LEX_DECLARATION);
	addChild(head, declaration);

	if(!isTokenType(VAR)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), VAR);
	consume();
	parseAssign(declaration);
}

void parseAssign(Lexeme* head){
	Lexeme* assign = createLexeme(LEX_ASSIGN);
	addChild(head, assign);

	if(!isTokenType(IDENTIFIER)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), IDENTIFIER);
	parseIdentifier(assign);

	if(!isTokenType(EQUALS)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), EQUALS);
	consume();
	parseExpression(assign);
}

//<func>			:= fn identifier ( <arglist> ) { <stmtlist> }
void parseFunc(Lexeme* head){
	Lexeme* func = createLexeme(LEX_FUNC);
	addChild(head, func);

	if(!isTokenType(FN)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), FN);
	consume();
	parseIdentifier(func);
	if(!isTokenType(PAREN_OPEN)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), PAREN_OPEN);
	consume();

	parseArgList(func);

	if(!isTokenType(PAREN_CLOSE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), PAREN_CLOSE);
	consume();
	if(!isTokenType(CURLY_OPEN)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), CURLY_OPEN);
	consume();

	parseStmtList(func);

	if(!isTokenType(CURLY_CLOSE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), CURLY_CLOSE);
	consume();
}


void parseReturn(Lexeme* head){

}

void parseFuncCall(Lexeme* head){

}

//expressionNonMath := ( <expression> ) | <assign> | <funcCall> | identifier | <logic>
void parseExpressionNonMath(Lexeme* head){
	Lexeme* expression = createLexeme(LEX_EXPRESSION_NONMATH);
	addChild(head, expression);

	if(isTokenType(PAREN_OPEN)){
		consume();
		parseExpression(expression);
		if(!isTokenType(PAREN_CLOSE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), PAREN_CLOSE);
		consume();
	}else if(isPeekType(EQUALS)){
		parseAssign(expression);
	}else if(isPeekType(PAREN_OPEN)){
		parseFuncCall(expression);
	}else if(isPeekType(AND) || isPeekType(OR) || isPeekType(NOT) || isPeekType(EQUALS_EQUALS) || isPeekType(LEQ)){
		parseLogic(expression);
	}else if(isTokenType(IDENTIFIER)) parseIdentifier(expression);
	else if(isTokenType(NUMBER)) parseNumber(expression);
}

//<expression>	:= <expressionNonMath> | <sub>
void parseExpression(Lexeme* head){
	Lexeme* expression = createLexeme(LEX_EXPRESSION);
	addChild(head, expression);

	if(isPeekType(MINUS)) parseSub(expression);
	else parseExpressionNonMath(expression);
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

//<sub>			:= <expressionNonMath> - <expression>
void parseSub(Lexeme* head){
	Lexeme* sub = createLexeme(LEX_SUB);
	addChild(head, sub);

	parseExpressionNonMath(sub);

	if(isTokenType(MINUS)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), MINUS);
	consume();

	parseExpression(sub);
}


void parseProgram(Lexeme* head){
	if(isTokenType(END)) ERR_UNEXPECTED_EOF(currentToken());
	if(isTokenType(UNKNOWN)) ERR_UNEXPECTED_TOKEN(currentToken());
	parseStmtList(head);

	if(isTokenType(END)) return;
	
	ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), END);
}

Lexeme* createLexeme(LexemeType type){
	Lexeme* lexeme = malloc(sizeof(Lexeme));
	lexeme->parent = lexeme->nextSibling = lexeme->prevSibling = lexeme->firstChild = NULL;
	lexeme->token = currentToken();
	lexeme->type = type;
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


void printAST(char* prefix, Lexeme* head){
	if(head == NULL) return;

	printf(prefix);

	Lexeme* single = head;
	while(single != NULL){
		printf("%s\t", single->type);
	}
	printf("\n");

	char buffer[256];
	sprintf(buffer, "%s==\0", prefix);

	printAST(buffer, head->firstChild);
}

Lexeme* parse(Arguments* args, Token* headToken){
	current = &headToken;
	Lexeme* head = createLexeme(LEX_PROGRAM);
	parseProgram(head);

	if(args->printAST) printAST("", head);

	return head;
}