#ifndef _ERROR_H_
	#define _ERROR_H_

	#include "lexer.h"

	#define LOG 0
	#define SEVERE 1

	// A typical error report. Expects the following parameters in order:
	// Type (defined above), line number, column number, message, any arguments to be passed to the print.
	#define ERR(T, L, C, M, ...) {error_log(T, "(line: %d, col: %d) " M, L, C, __VA_ARGS__);}

	void error_log(int type, char* errorMessage, ...);

#endif
