#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "error.h"
#include "lexer.h"

void error_log(int type, char *errorMessage, ...){
	va_list valist;
	va_start(valist, errorMessage);
	char *buffer = malloc(150);
	sprintf(buffer, "[Err]: %s\n", errorMessage);
	vprintf(buffer, valist);
	va_end(valist);
	free(buffer);
	if(type == SEVERE){
		printf("Closing due to error being severe\n");
		exit(1);
	}
}