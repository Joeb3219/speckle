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

	if(isTokenType(END) || isTokenType(CURLY_CLOSE) ) return;
	parseStmt(stmtList);
	if(!isTokenType(SEMICOLON)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), SEMICOLON);
	consume();

	parseStmtList(stmtList);
}

void parseFunctions(Lexeme* head){
	Lexeme* functions = createLexeme(LEX_FUNCTIONS);
	addChild(head, functions);

	if(isTokenType(END) || isTokenType(CURLY_CLOSE) ) return;
	parseFunc(functions);

	parseFunctions(functions);
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
	Lexeme* ret = createLexeme(LEX_RETURN);
	addChild(head, ret);

	if(!isTokenType(RET)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), RET);
	consume();

	parseExpression(ret);
}

void parseFuncCall(Lexeme* head){
	Lexeme* funcCall = createLexeme(LEX_FUNCCALL);
	addChild(head, funcCall);

	parseIdentifier(funcCall);

	if(!isTokenType(PAREN_OPEN)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), PAREN_OPEN);
	consume();

	parseParamList(funcCall);

	if(!isTokenType(PAREN_CLOSE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), PAREN_CLOSE);
	consume();
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
	}else if(isTokenType(AND) || isTokenType(OR) || isTokenType(NOT) || isTokenType(EQUALS_EQUALS) || isTokenType(LEQ)){
		parseLogic(expression);
	}else if(isTokenType(IDENTIFIER)) parseIdentifier(expression);
	else if(isTokenType(NUMBER)) parseNumber(expression);
}

//<expression>	:= <expressionNonMath> | <sub>
void parseExpression(Lexeme* head){
	Lexeme* expression = createLexeme(LEX_EXPRESSION);
	addChild(head, expression);


	if(isTokenType(MINUS)) parseSub(expression);
	else parseExpressionNonMath(expression);
}

// <logic>			:= <leq>
// <leq>			:= <= <expression> <equals> | <equals>
// <equals>		:= == <expresion> <or> | <or>
// <or>			:= | <expression> <and> | <and>
// <and>			:= & <expression> <not> | <not>
// <not>			:= ! <expression>
void parseLogic(Lexeme* head){
	Lexeme* logic = createLexeme(LEX_LOGIC);
	addChild(head, logic);

	parseLeq(logic);
}

void parseLeq(Lexeme* head){
	Lexeme* leq = createLexeme(LEX_LEQ);
	addChild(head, leq);

	if(isTokenType(LEQ)){
		consume();
		parseExpression(leq);
		parseEquals(leq);
	}else{
		parseEquals(leq);
	}
}

void parseEquals(Lexeme* head){
	Lexeme* equals = createLexeme(LEX_EQUALS);
	addChild(head, equals);

	if(isTokenType(EQUALS_EQUALS)){
		consume();
		parseExpression(equals);
		parseOr(equals);
	}else{
		parseOr(equals);
	}
}

void parseOr(Lexeme* head){
	Lexeme* or = createLexeme(LEX_OR);
	addChild(head, or);

	if(isTokenType(OR)){
		consume();
		parseExpression(or);
		parseAnd(or);
	}else{
		parseAnd(or);
	}
}

void parseAnd(Lexeme* head){
	Lexeme* and = createLexeme(LEX_AND);
	addChild(head, and);

	if(isTokenType(AND)){
		consume();
		parseExpression(and);
		parseNot(and);
	}else{
		parseNot(and);
	}
}

void parseNot(Lexeme* head){
	Lexeme* not = createLexeme(LEX_NOT);
	addChild(head, not);

	if(isTokenType(NOT)) consume(); 
	parseExpression(not);
}

void parseArgList(Lexeme* head){
	Lexeme* argList = createLexeme(LEX_ARGLIST);
	addChild(head, argList);

	if(isPeekType(COMMA)){
		parseIdentifier(argList);
		consume();
		parseArgList(argList);
	}else if(!isTokenType(PAREN_CLOSE)){
		parseIdentifier(argList);
	}
}

void parseParamList(Lexeme* head){
	Lexeme* paramList = createLexeme(LEX_ARGLIST);
	addChild(head, paramList);

	if(isPeekType(COMMA)){
		parseExpression(paramList);
		consume();
		parseParamList(paramList);
	}else if(!isTokenType(PAREN_CLOSE)){
		parseExpression(paramList);
	}
}

void parseIf(Lexeme* head){
	Lexeme* ifLex = createLexeme(LEX_WHILE);
	addChild(head, ifLex);

	if(!isTokenType(WHILE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), WHILE);
	consume();
	if(!isTokenType(PAREN_OPEN)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), PAREN_OPEN);
	consume();
	
	parseExpression(ifLex);

	if(!isTokenType(PAREN_CLOSE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), PAREN_CLOSE);
	consume();

	if(!isTokenType(CURLY_OPEN)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), CURLY_OPEN);
	consume();

	parseStmtList(ifLex);

	if(!isTokenType(CURLY_CLOSE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), CURLY_CLOSE);
	consume();
}

void parseWhile(Lexeme* head){
	Lexeme* whileLex = createLexeme(LEX_WHILE);
	addChild(head, whileLex);

	if(!isTokenType(WHILE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), WHILE);
	consume();
	if(!isTokenType(PAREN_OPEN)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), PAREN_OPEN);
	consume();

	parseExpression(whileLex);

	if(!isTokenType(PAREN_CLOSE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), PAREN_CLOSE);
	consume();

	if(!isTokenType(CURLY_OPEN)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), CURLY_OPEN);
	consume();

	parseStmtList(whileLex);

	if(!isTokenType(CURLY_CLOSE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), CURLY_CLOSE);
	consume();
}

//<sub>			:= <expressionNonMath> - <expression>
//<sub>			:= -<expression>
void parseSub(Lexeme* head){
	Lexeme* sub = createLexeme(LEX_SUB);
	addChild(head, sub);

	if(!isTokenType(MINUS)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), MINUS);
		
	consume();
		
	parseExpression(sub);
	
	parseExpression(sub);
}


void parseProgram(Lexeme* head){
	if(isTokenType(END)) ERR_UNEXPECTED_EOF(currentToken());
	if(isTokenType(UNKNOWN)) ERR_UNEXPECTED_TOKEN(currentToken());
	parseFunctions(head);

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
		case LEX_FUNCTIONS: return "FUNCTIONS";
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