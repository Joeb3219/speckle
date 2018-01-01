#ifndef _SPECKLE_H_
	#define _SPECKLE_H_

	struct Arguments{
		int help;
		int printTokens;
		int reconstruct;
		int printAST;
		char* outputName;
		char* inputFile;
	};

	typedef struct Arguments Arguments;

#endif
