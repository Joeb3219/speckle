#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "speckle.h"

#define MAX_TOKEN_SIZE 32

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

void identifyToken(Token* token){
	token->type = END;
}


void printToken(FILE* output, Token* token){
	fprintf(output, "{%d:%d, D: %s, T: %0X}\n", token->lineNo, token->colNo, "test", token->type);
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
	char c;
	int i = 0;
	int colNo = -1, lineNo = 0;
	Token* current = createToken();
	Token* head = current;

	while( (c = fgetc(file)) != EOF){
		colNo ++;

		// A carriage return is bringing us back to the beginning of the line.
		if(c == '\r'){
			colNo = -1;
			continue;
		}
		
		// Any spacing elements should be handled to create a new token.
		if(c == ' ' || c == '\t' || c == '\n'){
			if(c == '\n'){
				lineNo ++;
				colNo = -1;
			}

			identifyToken(current);

			i = 0;
			// Create a new token and link them up.
			current->next = createToken();
			current->next->prev = current;
			current = current->next;
			current->lineNo = lineNo;
			current->colNo = colNo;

			continue;			
		}
		current->data[i ++] = c;
	}

	identifyToken(current);
	if(current->type != END){
		current->next = createToken();
			current->next->prev = current;
			current = current->next;
			current->lineNo = lineNo;
			current->colNo = colNo;
			current->type = END;
	}

	if(args->printTokens) printTokens(stdout, head);

	return head;
}