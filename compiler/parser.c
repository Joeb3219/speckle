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

// Private function definitions
void parseIdentifier(Lexeme* head);
void parseNumber(Lexeme* head);
void parseProgram(Lexeme* head);
void parseStmtList(Lexeme* head);
void parseFunctions(Lexeme* head);
void parseStmt(Lexeme* head);
void parseDeclaration(Lexeme* head);
void parseAssign(Lexeme* head);
void parseFunc(Lexeme* head);
void parseReturn(Lexeme* head);
void parseFuncCall(Lexeme* head);
void parseExpression(Lexeme* head);
void parseLogic(Lexeme* head);
void parseLeq(Lexeme* head);
void parseEquals(Lexeme* head);
void parseOr(Lexeme* head);
void parseAnd(Lexeme* head);
void parseNot(Lexeme* head);
void parseArgList(Lexeme* head);
void parseParamList(Lexeme* head);
void parseIf(Lexeme* head);
void parseWhile(Lexeme* head);
void parseIdentOrNumber(Lexeme* head);
void parseMath(Lexeme* head);
void parseSub(Lexeme* head);
void parseMult(Lexeme* head);
void parseDiv(Lexeme* head);
void parseAdd(Lexeme* head);
void parseMod(Lexeme* head);	
void parseElement(Lexeme* head);
void parseArray(Lexeme* head);	

Token** current = NULL;

void addChild(Lexeme* parent, Lexeme* child){
	if(child == NULL || parent == NULL) return;
	child->parent = parent;

	Lexeme* lastSibling = parent->firstChild;
	if(lastSibling == NULL) parent->firstChild = child;
	else{
		while(lastSibling->nextSibling != NULL) lastSibling = lastSibling->nextSibling;
		lastSibling->nextSibling = child;
		child->prevSibling = lastSibling;
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

	parseFunctions(head);
}

//<stmt>	:= <func> | <declaration> | <expression> | <return> | <if>
void parseStmt(Lexeme* head){
	Lexeme* stmt = createLexeme(LEX_STMT);
	addChild(head, stmt);

	if(isTokenType(IF)) parseIf(stmt);
	else if(isTokenType(RET)) parseReturn(stmt);
	else if(isTokenType(FN)) parseFunc(stmt);
	else if(isTokenType(VAR)) parseDeclaration(stmt);
	else if(isTokenType(WHILE)) parseWhile(stmt);
	else parseExpression(stmt);
}

// <declaration>	:= var identifier | var identifier = <expression> | var identifier = <array>
void parseDeclaration(Lexeme* head){
	Lexeme* declaration = createLexeme(LEX_DECLARATION);
	addChild(head, declaration);

	if(!isTokenType(VAR)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), VAR);
	consume();
	
	// If the token following our identifier is an equals sign, then we will parse up to the assignment
	// Otherwise, we will just cut it off as a pure declaration.
	if(isPeekType(EQUALS)){
		if(!isTokenType(IDENTIFIER)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), IDENTIFIER);
		parseIdentifier(declaration);

		if(!isTokenType(EQUALS)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), EQUALS);
		consume();

		// If we find the next token to be an open curly brace, then we are attempting to create an array.
		if(isTokenType(CURLY_OPEN)) parseArray(declaration);
		else parseExpression(declaration);
	}else parseIdentifier(declaration);
}

// <element>		:= identifier { <identOrNumber> }
void parseElement(Lexeme* head){
	Lexeme* element = createLexeme(LEX_ELEMENT);
	addChild(head, element);

	parseIdentifier(element);

	if(!isTokenType(CURLY_OPEN)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), CURLY_OPEN);
	consume();	

	if(isTokenType(IDENTIFIER)) parseIdentifier(element);
	else if(isTokenType(NUMBER)) parseNumber(element);
	else ERR_UNEXPECTED_TOKEN(currentToken());

	if(!isTokenType(CURLY_CLOSE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), CURLY_CLOSE);
	consume();	
}

// <array>			:= { <identOrNumber> }
void parseArray(Lexeme* head){
	Lexeme* array = createLexeme(LEX_ARRAY);
	addChild(head, array);

	if(!isTokenType(CURLY_OPEN)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), CURLY_OPEN);
	consume();

	parseIdentOrNumber(array);

	if(!isTokenType(CURLY_CLOSE)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), CURLY_CLOSE);
	consume();	
}

// <assign>		:= identifier = <expression> | <element> = <expression> | identifier = <array> | <element> = <array>
void parseAssign(Lexeme* head){
	Lexeme* assign = createLexeme(LEX_ASSIGN);
	addChild(head, assign);

	if(isTokenType(IDENTIFIER) && !isPeekType(CURLY_OPEN)) parseIdentifier(assign);
	else if(isPeekType(CURLY_OPEN)) parseElement(assign);
	else ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), IDENTIFIER);

	if(!isTokenType(EQUALS)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), EQUALS);
	consume();

	// If we find the next token to be an open curly brace, then we are attempting to create an array.
	if(isTokenType(CURLY_OPEN)) parseArray(assign);
	else parseExpression(assign);
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

// <expression>	:= <math> | <logic> | <identOrNumber> | <funcCall>
void parseExpression(Lexeme* head){
	Lexeme* expression = createLexeme(LEX_EXPRESSION);
	addChild(head, expression);

	if(isPeekType(MINUS) || isPeekType(PLUS) || isPeekType(TIMES) || isPeekType(DIV) || isPeekType(MOD)) parseMath(expression);
	else if(isPeekType(LEQ) || isPeekType(AND) || isPeekType(OR) || isTokenType(NOT) || isPeekType(EQUALS_EQUALS)) parseLogic(expression);
	else if(isPeekType(PAREN_OPEN)) parseFuncCall(expression);
	else if((isTokenType(IDENTIFIER) && isPeekType(CURLY_OPEN))){
		// TODO: fix this hellhole 1/10/18
		if(currentToken()->next->next->next->next->type == EQUALS) parseAssign(expression);
		else parseElement(expression);
	}else if(isPeekType(EQUALS)) parseAssign(expression);
	else parseIdentOrNumber(expression);
}


void parseIdentOrNumber(Lexeme* head){
	if(isPeekType(CURLY_OPEN)) parseElement(head);
	else if(isTokenType(IDENTIFIER)) parseIdentifier(head);
	else if(isTokenType(NUMBER)) parseNumber(head);
}

void parseLogic(Lexeme* head){
	Lexeme* logic = createLexeme(LEX_LOGIC);
	addChild(head, logic);

	if(isPeekType(LEQ)) parseLeq(logic);
	else if(isPeekType(OR)) parseOr(logic);
	else if(isPeekType(AND)) parseAnd(logic);
	else if(isPeekType(EQUALS_EQUALS)) parseEquals(logic);
	else if(isTokenType(NOT)) parseNot(logic);
}

void parseLeq(Lexeme* head){
	Lexeme* leq = createLexeme(LEX_LEQ);
	addChild(head, leq);

	parseIdentOrNumber(leq);

	if(!isTokenType(LEQ)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), LEQ);
	consume();

	parseIdentOrNumber(leq);
}

void parseEquals(Lexeme* head){
	Lexeme* equals = createLexeme(LEX_EQUALS);
	addChild(head, equals);

	parseIdentOrNumber(equals);

	if(!isTokenType(EQUALS_EQUALS)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), EQUALS_EQUALS);
	consume();

	parseIdentOrNumber(equals);
}

void parseOr(Lexeme* head){
	Lexeme* or = createLexeme(LEX_OR);
	addChild(head, or);

	parseIdentOrNumber(or);

	if(!isTokenType(OR)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), OR);
	consume();

	parseIdentOrNumber(or);
}

void parseAnd(Lexeme* head){
	Lexeme* and = createLexeme(LEX_AND);
	addChild(head, and);
	
	parseIdentOrNumber(and);

	if(!isTokenType(AND)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), AND);
	consume();

	parseIdentOrNumber(and);
}

void parseNot(Lexeme* head){
	Lexeme* not = createLexeme(LEX_NOT);
	addChild(head, not);

	if(isTokenType(NOT)) consume(); 

	parseIdentOrNumber(not);
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
	Lexeme* ifLex = createLexeme(LEX_IF);
	addChild(head, ifLex);

	
	if(!isTokenType(IF)) ERR_UNEXPECTED_TOKEN_EXPECTED(currentToken(), IF);
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

void parseMath(Lexeme* head){
	Lexeme* math = createLexeme(LEX_MATH);
	addChild(head, math);

	if(isPeekType(MINUS)) parseSub(math);
	else if(isPeekType(PLUS)) parseAdd(math);
	else if(isPeekType(TIMES)) parseMult(math);
	else if(isPeekType(DIV)) parseDiv(math);
	else if(isPeekType(MOD)) parseMod(math);
}


void parseSub(Lexeme* head){
	Lexeme* sub = createLexeme(LEX_SUB);
	addChild(head, sub);

	if(!isPeekType(MINUS)) ERR_UNEXPECTED_TOKEN_EXPECTED(peek(), MINUS);
		
	parseIdentOrNumber(sub);

	consume();
	
	parseIdentOrNumber(sub);
}


void parseAdd(Lexeme* head){
	Lexeme* add = createLexeme(LEX_ADD);
	addChild(head, add);

	if(!isPeekType(PLUS)) ERR_UNEXPECTED_TOKEN_EXPECTED(peek(), PLUS);
		
	parseIdentOrNumber(add);

	consume();
	
	parseIdentOrNumber(add);
}


void parseMult(Lexeme* head){
	Lexeme* mult = createLexeme(LEX_MULT);
	addChild(head, mult);

	if(!isPeekType(TIMES)) ERR_UNEXPECTED_TOKEN_EXPECTED(peek(), TIMES);
		
	parseIdentOrNumber(mult);

	consume();
	
	parseIdentOrNumber(mult);
}


void parseDiv(Lexeme* head){
	Lexeme* div = createLexeme(LEX_DIV);
	addChild(head, div);

	if(!isPeekType(DIV)) ERR_UNEXPECTED_TOKEN_EXPECTED(peek(), DIV);
		
	parseIdentOrNumber(div);

	consume();
	
	parseIdentOrNumber(div);
}

void parseMod(Lexeme* head){
	Lexeme* mod = createLexeme(LEX_MOD);
	addChild(head, mod);

	if(!isPeekType(MOD)) ERR_UNEXPECTED_TOKEN_EXPECTED(peek(), MOD);
		
	parseIdentOrNumber(mod);

	consume();
	
	parseIdentOrNumber(mod);
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

void printNode(Lexeme* node){
	if(node == NULL) printf("NULL NODE");
	else printf("%s: %s\t", lexemeTypeToChar(node->type), node->token->data);
}

void printAST(char* prefix, Lexeme* head){
	if(head == NULL) return;

	printf(prefix);
	printNode(head);
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
		case LEX_MATH: return "MATH";
		case LEX_ADD: return "ADD";
		case LEX_MULT: return "MULT";
		case LEX_DIV: return "DIV";
		case LEX_IDENTORNUMBER: return "IDENT_OR_NUMBER";
		case LEX_FUNCTIONS: return "FUNCTIONS";
		case LEX_ELEMENT: return "ELEMENT";
		case LEX_ARRAY: return "ARRAY";
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


void condenseTree(Lexeme* head){
	Lexeme* child = head->firstChild;
	switch(head->type){
		case LEX_PROGRAM:
			while(child != NULL){
				condenseTree(child);
				child = child->nextSibling;
			}
			break;
		case LEX_FUNCTIONS:
			break;
			
	}
}