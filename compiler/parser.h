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

	int parseProgram(Lexeme* head, Token** current);
	int parseStmtList(Lexeme* head, Token** current);
	int parseStmt(Lexeme* head, Token** current);
	int parseDeclaration(Lexeme* head, Token** current);
	int parseAssign(Lexeme* head, Token** current);
	int parseFunc(Lexeme* head, Token** current);
	int parseFuncCall(Lexeme* head, Token** current);
	int parseExpression(Lexeme* head, Token** current);
	int parseLogic(Lexeme* head, Token** current);
	int parseLeq(Lexeme* head, Token** current);
	int parseEquals(Lexeme* head, Token** current);
	int parseOr(Lexeme* head, Token** current);
	int parseAnd(Lexeme* head, Token** current);
	int parseNot(Lexeme* head, Token** current);
	int parseArgList(Lexeme* head, Token** current);
	int parseParamList(Lexeme* head, Token** current);
	int parseIf(Lexeme* head, Token** current);
	int parseWhile(Lexeme* head, Token** current);
	int parseSub(Lexeme* head, Token** current);
	
	void addChild(Lexeme* parent, Lexeme* child);
	void destroyTree(Lexeme* head);
	Lexeme* createLexeme();
	void destroyLexeme(Lexeme* lexeme);
	Lexeme* parse(Arguments* args, Token* headToken);

#endif