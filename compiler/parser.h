#ifndef _PARSER_H_
	#define _PARSER_H_

	#include "lexer.h"

	enum LexemeType{
		LEX_PROGRAM, LEX_STMTLIST, LEX_STMT,
		LEX_DECLARATION, LEX_ASSIGN, 
		LEX_FUNC, LEX_FUNCCALL, LEX_EXPRESSION, LEX_RETURN,
		LEX_LOGIC, LEX_LEQ, LEX_EQUALS, LEX_OR, LEX_AND, LEX_NOT,
		LEX_ARGLIST, LEX_PARAMLIST,
		LEX_IF, LEX_WHILE, LEX_SUB, LEX_ADD, LEX_MULT, LEX_DIV, LEX_MOD,
		LEX_IDENTIFIER, LEX_NUMBER,
		LEX_MATH, LEX_IDENTORNUMBER, LEX_FUNCTIONS,
		LEX_ELEMENT, LEX_ARRAY
	};

	typedef enum LexemeType LexemeType;

	struct Lexeme{
		struct Lexeme* firstChild;
		struct Lexeme* parent;
		struct Lexeme* nextSibling;
		struct Lexeme* prevSibling;
		Token* token;
		LexemeType type;
	};

	typedef struct Lexeme Lexeme;

	char* lexemeTypeToChar(LexemeType type);

	void condenseTree(Lexeme* head);
	void printNode(Lexeme* node);
	void printAST(char* prefix, Lexeme* head);
	void addChild(Lexeme* parent, Lexeme* child);
	void destroyTree(Lexeme* head);
	Lexeme* createLexeme();
	void destroyLexeme(Lexeme* lexeme);
	Lexeme* parse(Arguments* args, Token* headToken);

#endif