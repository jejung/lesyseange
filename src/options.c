#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lesyange.h"

options_t parse_args(int argc, char* argv[]) 
{
	options_t r;
	r.ebnf_file = NULL;
	r.h = 0;
	r.v = 0;
	if (argc == 1) 
    {
		r.h = 1;
		return r;
	}
	short i;
	for (i = 1; i < argc; i++)  
    {
		char* arg = argv[i];
		if (*arg == '-') 
        {
			arg++;
			if (*arg == 'h') 
            {
				r.h = 1;
				break;
			} else if (*arg == 'v') 
            {
				r.v = 1;
				break;
			} else if (*arg == 'd') 
            {
				r.d = 1;
			} else if (*arg == 'c') 
			{
			    r.c = 1;
			}
		} else 
			r.ebnf_file = argv[i];	
	}
	return r;
}

void print_options(OPT_CALL) 
{
	printf("\nlesyange version %s called with options:\n", VERSION);
	printf("\tebnf_file: %s\n", opt.ebnf_file);
	if (opt.d) 
    {
		printf("\twith debug output enabled\n");
	}
    printf("\n");
}

void print_usage(void) 
{
	printf("\nValid usage for lesyange\n");
	printf("\t lesyange [-OPTION1, -OPTION2, ...]  ebnf_file\n"); 
	printf("OPTIONS:\n");
	printf("\t -h: print this help text and exit.\n");
	printf("\t -v: print the lesyange version and exit.\n");
	printf("\t -d: print debug information into the parse process\n");
	printf("\t -c: cat the contents of the input file and exit\n");
    printf("\n");
	exit(0);
}

void print_version() 
{
	printf("\nlesyange version %s\n\n", VERSION);
}
