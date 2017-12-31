#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "speckle.h"

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

// Returns a struct pointer to an Arguments struct representing the entire arguments list the user passed.
// This function handles setting every flag, including default values if not set, etc.
// Will return NULL on error.
Arguments* getArguments(int argc, char** argv){
	Arguments* args = malloc(sizeof(Arguments));

	args->help = flagSet("-h") || flagSet("--help");

	return args;
}

int main(int argc, char** argv){
	Arguments* args = getArguments(argc, argv);

	if(args == NULL || args->help){
		printf("Usage: %s <parameters>\n", argv[0]);
		printf("Parameters:\n");
		printf("\t-h:\tHelp, prints this dialog\n");
		printf("\t-o:\tOutput filename\n");
	}

	free(args);

	return 0;
}
