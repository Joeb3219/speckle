#ifndef _PARSER_H_
	#define _PARSER_H_

	#include "lexer.h"

	enum LexemeType{
		LEX_PROGRAM, LEX_STMTLIST, LEX_STMT,
		LEX_DECLARATION, LEX_ASSIGN, 
		LEX_FUNC, LEX_FUNCCALL, LEX_EXPRESSION,
		LEX_LOGIC, LEX_LEQ, LEX_EQUALS, LEX_OR, LEX_AND, LEX_NOT,
		LEX_ARGLIST, LEX_PARAMLIST,
		LEX_IF, LEX_WHILE, LEX_SUB
	};

	typedef enum LexemeType LexemeType;

	struct Lexeme{
		struct Lexeme* firstChild;
		struct Lexeme* parent;
		struct Lexeme* nextSibling;
		struct Lexeme* prevSibling;
		Token* token;
	};

	typedef struct Lexeme Lexeme;

	void parseProgram(Lexeme* head, Token** current);

	void addChild(Lexeme* parent, Lexeme* child);
	void destroyTree(Lexeme* head);
	Lexeme* createLexeme();
	void destroyLexeme(Lexeme* lexeme);
	Lexeme* parse(Arguments* args, Token* headToken);

#endif