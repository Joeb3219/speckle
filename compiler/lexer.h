#ifndef _LEXER_H_
	#define _LEXER_H_

	#include "speckle.h"

	enum TokenType{
		CURLY_OPEN, CURLY_CLOSE, PAREN_OPEN, PAREN_CLOSE,
		SEMICOLON, EQUALS_EQUALS, EQUALS, AND, OR, LEQ, NOT,
		MINUS, PLUS, TIMES, DIV, RET, FN, VAR, IF,
		IDENTIFIER, NUMBER, UNKNOWN, END, COMMA, CHARACTER_TEMPORARY
	};

	typedef enum TokenType TokenType;

	struct Token{
		int lineNo;
		int colNo;
		char* data;
		TokenType type;
		struct Token* next;
		struct Token* prev;
	};

	typedef struct Token Token;

	char* typeToText(TokenType type);
	int isIdentifier(Token* token);
	int isNumber(Token* token);
	void printToken(FILE* output, Token* token);
	void printTokens(FILE* output, Token* head);
	void reconstructTokens(FILE* output, Token* head);
	void identifyToken(Token* token, FILE* file);
	Token* tokenize(Arguments* args, FILE* file);
	Token* createToken();
	void freeToken(Token* token);


#endif