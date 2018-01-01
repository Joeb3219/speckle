#ifndef _PARSER_H_
	#define _PARSER_H_

	#include "lexer.h"

	enum LexemeType{
		LEX_PROGRAM, LEX_STMTLIST, LEX_STMT,
		LEX_DECLARATION, LEX_ASSIGN, 
		LEX_FUNC, LEX_FUNCCALL, LEX_EXPRESSION, LEX_RETURN,
		LEX_LOGIC, LEX_LEQ, LEX_EQUALS, LEX_OR, LEX_AND, LEX_NOT,
		LEX_ARGLIST, LEX_PARAMLIST,
		LEX_IF, LEX_WHILE, LEX_SUB,
		LEX_IDENTIFIER, LEX_NUMBER,
		LEX_EXPRESSION_NONMATH
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

	void parseIdentifier(Lexeme* head);
	void parseNumber(Lexeme* head);
	void parseProgram(Lexeme* head);
	void parseStmtList(Lexeme* head);
	void parseStmt(Lexeme* head);
	void parseDeclaration(Lexeme* head);
	void parseAssign(Lexeme* head);
	void parseFunc(Lexeme* head);
	void parseReturn(Lexeme* head);
	void parseFuncCall(Lexeme* head);
	void parseExpressionNonMath(Lexeme* head);
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
	void parseSub(Lexeme* head);
	
	void printAST(char* prefix, Lexeme* head);
	void addChild(Lexeme* parent, Lexeme* child);
	void destroyTree(Lexeme* head);
	Lexeme* createLexeme();
	void destroyLexeme(Lexeme* lexeme);
	Lexeme* parse(Arguments* args, Token* headToken);

#endif