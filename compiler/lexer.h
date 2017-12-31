#ifndef _LEXER_H_
	#define _LEXER_H_

	#include "speckle.h"

	enum TokenType{
		CURLY_OPEN, CURLY_CLOSE, PAREN_OPEN, PAREN_CLOSE,
		SEMICOLON, EQUALS_EQUALS, EQUALS, AND, OR,
		MINUS, RET, FN, VAR, IF, WHILE,
		IDENTIFIER, NUMBER, UNKNOWN
	};

	typedef enum TokenType TokenType;

	struct Token{
		int lineNo;
		int colNo;
		char* data;
		TokenType type;
	};

	typedef struct Token Token;

	Token** tokenize(Arguments* args, FILE* file);

#endif