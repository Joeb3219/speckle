#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "speckle.h"
#include "lexer.h"

#define flagSet(A) (isFlagSet(argc, argv, A))
#define getArgOrDefault(A, B) ((getArgumentFollowingFlag(argc, argv, A) == NULL) ? B : getArgumentFollowingFlag(argc, argv, A))

// Given a stream of arguments: -a -b string -c ..., will return string if given -b, or -b if given -a.
// Effectively, returns the next argument following a given string.
// If the flag isn't found, or if it's the last argument, will return NULL.
char* getArgumentFollowingFlag(int argc, char** argv, char* flag){
        int i;
	// We only scan up to argc - 1 because if the flag occurs at position argc - 1, there can't be an argument following it.
	// That is, if the argument list is: -a -b -c -d, asking for -c should yield -d, but asking for -d doesn't have an argument following it.
        for(i = 0; i < argc - 1; i ++){
                if(strcmp(argv[i], flag) == 0) return argv[i + 1];
        }
        return NULL;
}

// Returns 1 if the flag is present in the argument list, or 0 otherwise.
int isFlagSet(int argc, char** argv, char* flag){
        int i;
        for(i = 0; i < argc; i ++){
                if(strcmp(argv[i], flag) == 0) return 1;
        }
        return 0;
}

void printArguments(Arguments* args){
	printf("=============================================\n");
	printf("=             Speckle Arguments             =\n");
	printf("=============================================\n");
	printf("Help flag:\t\t%d\n", args->help);
	printf("Print Tokens:\t\t%d\n", args->printTokens);
	printf("Output Name:\t\t%s\n", args->outputName);
	printf("Input File:\t\t%s\n", args->inputFile);
	printf("=============================================\n");
	printf("=           End Speckle Arguments           =\n");
	printf("=============================================\n");
}

// Returns a struct pointer to an Arguments struct representing the entire arguments list the user passed.
// This function handles setting every flag, including default values if not set, etc.
// Will return NULL on error.
Arguments* getArguments(int argc, char** argv){
	Arguments* args = malloc(sizeof(Arguments));

	args->help = flagSet("-h") || flagSet("--help");
	args->printTokens = flagSet("-t") || flagSet("--tokens");
	args->outputName = getArgOrDefault("-o", "a.out");
	args->inputFile = getArgOrDefault("-f", NULL);
	args->reconstruct = flagSet("-r") || flagSet("--reconstruct");

	return args;
}

int main(int argc, char** argv){
	Arguments* args = getArguments(argc, argv);

	if(args == NULL || args->help || args->inputFile == NULL){
		printf("Usage: %s -f <file> <parameters>\n", argv[0]);
		printf("Parameters:\n");
		printf("\t-h:\tHelp, prints this dialog\n");
		printf("\t-t:\tPrint tokens, if set will print out the parsed tokens.\n");
		printf("\t-o:\tOutput filename\n");
		printf("\t-f:\tInput file\n");
		printf("\t-r:\tReconstruct Source, if set will reconstruct source from tokens and the AST separately.\n");
	}

	printArguments(args);

	FILE* file = fopen(args->inputFile, "r");
	Token* head = tokenize(args, file);
	fclose(file);

	free(args);

	return 0;
}
