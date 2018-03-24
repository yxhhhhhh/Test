#include <stdio.h>
#include <string.h>
#include "cmd_printf.h"
#include "CLI.h"

#define DEBUG 1

//#define DEBUG
void PrintfArgUsage(void)
{
	printf(" Usage: printf_arg [String]\n");
	printf(" Example: \n");
	printf(" printf_arg HelloWorld \n");
	
};

int32_t cmd_printf_hello(int argc,char* argv[])
{
	printf("Hello\n");
	return cliPASS;
}

int32_t cmd_printf_arg(int argc,char* argv[])
{
	if(argc<2)
	{
		PrintfArgUsage();
		return cliFAIL;	
	}
	
	printf("%s\n",argv[1]);
	return cliPASS;
}
