#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "error.h"
#include "speckle.h"

#define MAX_TOKEN_SIZE 32
#define LONGEST_STATIC_TOKEN 5

Token* createToken(){
	Token* token = malloc(sizeof(Token));
	token->lineNo = token->colNo = 0;
	token->data = malloc((MAX_TOKEN_SIZE + 1) * sizeof(char));
	token->type = 0;
	token->next = token->prev = NULL;
	return token;
}

void freeToken(Token* token){
	if(token->data != NULL) free(token->data);
	free(token);
}

void identifyToken(Token* token, FILE* file){
	if(token == NULL) return;
	if(token->data == NULL || strlen(token->data) == 0) token->type = END;
	
	size_t originalSize = strlen(token->data);

	TokenType type = -1;

	// Test if it's a number or an identifier
	while( type == -1 && strlen(token->data) > LONGEST_STATIC_TOKEN){
		if(isNumber(token)) type = NUMBER;
		else if(isIdentifier(token)) type = IDENTIFIER;
		else token->data[strlen(token->data) - 1] = '\0';
	}

	if( type == -1 && strlen(token->data) == 5 ){
		if(strcmp("while", token->data) == 0) type = WHILE;
		else if(isNumber(token)) type = NUMBER;
		else if(isIdentifier(token)) type = IDENTIFIER;
		else token->data[strlen(token->data) - 1] = '\0';
	}

	if( type == -1 && strlen(token->data) == 4 ){
		if(isNumber(token)) type = NUMBER;
		else if(isIdentifier(token)) type = IDENTIFIER;
		else token->data[strlen(token->data) - 1] = '\0';
	}


	if( type == -1 && strlen(token->data) == 3 ){
		if(token->data[0] == '\'' && token->data[2] == '\'') type = CHARACTER_TEMPORARY;
		else if(strcmp("ret", token->data) == 0) type = RET;
		else if(strcmp("var", token->data) == 0) type = VAR;
		else if(isNumber(token)) type = NUMBER;
		else if(isIdentifier(token)) type = IDENTIFIER;
		else token->data[strlen(token->data) - 1] = '\0';
	}


	if( type == -1 && strlen(token->data) == 2 ){
		if(strcmp("fn", token->data) == 0) type = FN;
		else if(strcmp("if", token->data) == 0) type = IF;
		else if(strcmp("<=", token->data) == 0) type = LEQ;
		else if(strcmp(">=", token->data) == 0) type = GEQ;
		else if(strcmp("==", token->data) == 0) type = EQUALS_EQUALS;
		else if(isNumber(token)) type = NUMBER;
		else if(isIdentifier(token)) type = IDENTIFIER;
		else token->data[strlen(token->data) - 1] = '\0';
	}

	if( type == -1 && strlen(token->data) == 1){
		if(strcmp("{", token->data) == 0) type = CURLY_OPEN;
		else if(strcmp("}", token->data) == 0) type = CURLY_CLOSE;
		else if(strcmp("(", token->data) == 0) type = PAREN_OPEN;
		else if(strcmp(")", token->data) == 0) type = PAREN_CLOSE;
		else if(strcmp(";", token->data) == 0) type = SEMICOLON;
		else if(strcmp("=", token->data) == 0) type = EQUALS;
		else if(strcmp("!", token->data) == 0) type = NOT;
		else if(strcmp("&", token->data) == 0) type = AND;
		else if(strcmp("|", token->data) == 0) type = OR;
		else if(strcmp("-", token->data) == 0) type = MINUS;
		else if(strcmp("+", token->data) == 0) type = PLUS;
		else if(strcmp("*", token->data) == 0) type = TIMES;
		else if(strcmp("/", token->data) == 0) type = DIV;
		else if(strcmp("<", token->data) == 0) type = LESS;
		else if(strcmp(">", token->data) == 0) type = GREATER;
		else if(strcmp("%", token->data) == 0) type = MOD;
		else if(strcmp(",", token->data) == 0) type = COMMA;
		else if(isNumber(token)) type = NUMBER;
		else if(isIdentifier(token)) type = IDENTIFIER;
		else token->data[strlen(token->data) - 1] = '\0';
	}

	if( type == -1 ) type = UNKNOWN;

	// Now that we've possibly subtracted some bytes from the data segment, we will go ahead and move the file pointer back that many bytes.
	originalSize -= strlen(token->data) - 1;
	fseek(file, -originalSize, SEEK_CUR);
	token->type = type;

}


char* typeToText(TokenType type){
	switch(type){
		case CURLY_OPEN: 	return "CURLY OPEN";
		case CURLY_CLOSE:	return "CURLY CLOSE";
		case PAREN_OPEN: 	return "PAREN OPEN";
		case PAREN_CLOSE: 	return "PAREN CLOSE";
		case SEMICOLON: 	return "SEMICOLON";
		case EQUALS_EQUALS: return "EQUALS EQUALS";
		case EQUALS: 		return "EQUALS";
		case AND: 			return "AND";
		case OR: 			return "OR";
		case LEQ: 			return "LEQ";
		case GREATER: 		return "GREATER";
		case LESS: 			return "LESS";
		case GEQ: 			return "GEQ";
		case NOT: 			return "NOT";
		case MINUS: 		return "MINUS";
		case PLUS: 			return "PLUS";
		case TIMES: 		return "TIMES";
		case DIV: 			return "DIV";
		case MOD: 			return "MOD";
		case RET: 			return "RET";
		case FN: 			return "FN";
		case VAR: 			return "VAR";
		case IF: 			return "IF";
		case WHILE: 			return "WHILE";
		case IDENTIFIER: 	return "IDENT";
		case NUMBER: 		return "NUMBER";
		case UNKNOWN: 		return "UNKNOWN";
		case END: 			return "END";
		case COMMA:			return "COMMA";
		default: 			return "ERROR";
	}
}

int isIdentifier(Token* token){
	int i;
	char c;
	for(i = 0; i < strlen(token->data); i ++){
		c = token->data[i];
		if(i == 0 && !( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) return 0;
		else if(!( (c >= '0' && c <= '9') || (c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) return 0;
	}
	return 1;
}

// Examples of valid numbers: +123, -123, 123, 1234
// Examples of invalid numbers: 1sd4, -+2, -, +, 34+, +a
int isNumber(Token* token){
	int i = 0;
	int start = 0;
	if(token == NULL || token->data == NULL || strlen(token->data) == 0) return 0;
	if(token->data[0] == '+' || token->data[0] == '-'){
		if(strlen(token->data) == 1) return 0;
		start = 1;
	}
	for(i = start; i < strlen(token->data); i ++){
		if(token->data[i] < '0' || token->data[i] > '9') return 0;
	}
	return 1;
}


void reconstructTokens(FILE* output, Token* head){
	fprintf(output, "RECONSTRUCTING\n");
	int currentLine = 0;
	printf("[0]:");
	while(head != NULL){
		while(currentLine < head->lineNo){
			fprintf(output, "\n[%d]: ", currentLine + 1);
			currentLine ++;
		}
		fprintf(output, "%s ", head->data);
		head = head->next;
	}
	fprintf(output, "\n");
	fprintf(output, "END RECONSTRUCTING\n");
}

void printToken(FILE* output, Token* token){
	fprintf(output, "{%d:%d, D: %s, T: %s}\n", token->lineNo, token->colNo, token->data, typeToText(token->type));
}

void printTokens(FILE* output, Token* head){
	fprintf(output, "TOKEN STREAM\n");
	while(head != NULL){
		printToken(output, head);
		head = head->next;
	}
	fprintf(output, "END TOKEN STREAM\n");
}

// Our tokenizer will work as such:
// Given that a token can be at most MAX_TOKEN_SIZE, we read in one character at a time up to MAX_TOKEN_SIZE.
// If we encounter a space element, we will break early. Otherwise, we continually add characters to our current
// token until we read a full buffer or whitespace.
// Next, we go pass the read string through from the end and narrow down until we have a token of a valid size.
Token* tokenize(Arguments* args, FILE* file){
	char c, j;
	int prevWasSpace = 0;
	int i = 0;
	int colNo = -1, lineNo = 0;
	Token* current = createToken();
	Token* head = current;


	while( (c = fgetc(file)) != EOF){
		colNo ++;
		if(prevWasSpace == 1) current->colNo = colNo;

		// A carriage return is bringing us back to the beginning of the line.
		if(c == '\r'){
			colNo = -1;
			continue;
		}
		
		// Any spacing elements should be handled to create a new token.
		if(i == 31 || c == ' ' || c == '\t' || c == '\n'){
			prevWasSpace = 1;

			j = fgetc(file);
                        ungetc(j, file);

			// If this entry is a space, and the prev is a "'", and the next is a "'", then we are dealing with a space character.
			if(i == 1 && current->data[0] == '\''){
				if(c == ' ' && j == '\''){
					colNo ++;
					current->data[i++] = ' ';
					current->data[i++] = fgetc(file);
					current->type = NUMBER;
					char* newString = malloc(sizeof(char) * 8);
	                                sprintf(newString, "%d", (int) current->data[1]);
	                                free(current->data);
       		                        current->data = newString;
					current->next = createToken();
		                        current->next->prev = current;
		                        current = current->next;
        		                current->lineNo = lineNo;
        		                current->colNo = colNo + 1;
					i = 0;
				}
			}

			if(i == 0){
				if(c == '\n' && j == '\n'){
					lineNo ++;
				}
				continue;
			}

			current->data[i++] = '\0';
			identifyToken(current, file);
			colNo -= (i - strlen(current->data));

			if(current->type == CHARACTER_TEMPORARY){
				current->type = NUMBER;
				char* newString = malloc(sizeof(char) * 8);
				sprintf(newString, "%d", (int) current->data[1]);
				free(current->data);
				current->data = newString;
			}

			// identifyToken may move back the file pointer.
			// If it did, and we are no longer at a new line, we don't have to increment the lineNo anymore.
			c = fgetc(file);
			ungetc(c, file);
			if(c == '\n'){
				lineNo ++;
				colNo = -1;
			}

			// Create a new token and link them up.
			current->next = createToken();
			current->next->prev = current;
			current = current->next;
			current->lineNo = lineNo;
			current->colNo = colNo + 1;
		
			i = 0;
			continue;	
		}
		prevWasSpace = 0;
		current->data[i ++] = c;
	}

	if( i != 0 ){
		current->data[i++] = '\0';
		identifyToken(current, file);
	}else{
		Token* old = current;
		if(current != NULL && current->prev != NULL) current = current->prev;
		freeToken(old);
	}
	
	if(current->type != END){
		current->next = createToken();
			current->next->prev = current;
			current = current->next;
			current->lineNo = lineNo;
			current->colNo = colNo;
			current->type = END;
	}

	if(args->printTokens) printTokens(stdout, head);
	if(args->reconstruct) reconstructTokens(stdout, head);

	return head;
}
