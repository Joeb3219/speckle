#ifndef _ERROR_H_
	#define _ERROR_H_

	#include "lexer.h"

	#define LOG 0
	#define SEVERE 1

	// A typical error report. Expects the following parameters in order:
	// Type (defined above), line number, column number, message, any arguments to be passed to the print.
	#define ERR(T, L, C, M, ...) {error_log(T, "(line: %d, col: %d) " M, L, C, __VA_ARGS__);}
	#define ERR_NO_ARGS(T, L, C, M) {error_log(T, "(line: %d, col: %d) " M, L, C);}

	// An error for an unexpected token. Expects the following parameters in order:
	// Token that offended, expected type.
	#define ERR_UNEXPECTED_TOKEN_EXPECTED(T, E) {ERR(SEVERE, T->lineNo, T->colNo, "Unexpected token, expected %s but found %s", typeToText(E), T->data)}

	// An error for an unexpected token. Expects the following parameters in order:
	// Token that offended, expected type.
	#define ERR_UNEXPECTED_TOKEN(T) {ERR(SEVERE, T->lineNo, T->colNo, "Unexpected token: %s", T->data)}

	// An error for unexepected end of file.
	#define ERR_UNEXPECTED_EOF(T) {ERR_NO_ARGS(SEVERE, T->lineNo, T->colNo, "Unexpected end of file.")}

	void error_log(int type, char* errorMessage, ...);

#endif
