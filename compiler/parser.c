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

	if(isPeekType(EQUALS)){
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


	if(isTokenType(PAREN_OPEN)){
		consume();
		parseExpression(expression);
		if(!isTokenType(PAREN_CLOSE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), PAREN_CLOSE);
		consume();
	}else if(isPeekType(MINUS)) parseSub(expression);
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
//<sub>			:= -<expression>
void parseSub(Lexeme* head){
//	printAST("--", head->parent->parent->parent->parent->parent->parent->parent);
	Lexeme* sub = createLexeme(LEX_SUB);
	addChild(head, sub);

	if(isTokenType(MINUS)){
		consume();
		parseExpression(sub);
	}else{
		parseExpressionNonMath(sub);

		if(!isTokenType(MINUS)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), MINUS);
		consume();

		parseExpression(sub);
	}
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
	printf("%s: %s\t", lexemeTypeToChar(head->type), head->token->data);
	printf("\n");

	char buffer[256];
	sprintf(buffer, "%s--\0", prefix);
	printAST(buffer, head->firstChild);

	Lexeme* single = head->nextSibling;
	while(single != NULL){
		printAST(prefix, single);
		single = single->nextSibling;
	}
}


char* lexemeTypeToChar(LexemeType type){
	switch(type){
		case LEX_PROGRAM: return "PROGRAM";
		case LEX_STMTLIST: return "STMTLIST";
		case LEX_STMT: return "STMT";
		case LEX_DECLARATION: return "DECLARATION";
		case LEX_ASSIGN: return "ASSIGN"; 
		case LEX_FUNC: return "FUNC";
		case LEX_FUNCCALL: return "FUNCCALL";
		case LEX_EXPRESSION: return "EXPRESSION";
		case LEX_RETURN: return "RETURN";
		case LEX_LOGIC: return "LOGIC";
		case LEX_LEQ: return "LEQ";
		case LEX_EQUALS: return "EQUALS";
		case LEX_OR: return "OR";
		case LEX_AND: return "AND";
		case LEX_NOT: return "NOT";
		case LEX_ARGLIST: return "ARGLIST";
		case LEX_PARAMLIST: return "PARAMLIST";
		case LEX_IF: return "IF";
		case LEX_WHILE: return "WHILE";
		case LEX_SUB: return "SUB";
		case LEX_IDENTIFIER: return "IDENTIFIER";
		case LEX_NUMBER: return "NUMBER";
		case LEX_EXPRESSION_NONMATH: return "EXPRESSION_NONMATH";
		default: return "ERROR";
	}
}

Lexeme* parse(Arguments* args, Token* headToken){
	current = &headToken;
	Lexeme* head = createLexeme(LEX_PROGRAM);
	parseProgram(head);

	if(args->printAST) printAST("", head);

	return head;
}