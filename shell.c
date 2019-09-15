/*
 *  COP4610 - Assignment 1
 *  Authors: Keaun Moughari, Hayden Rogers, Marlan McInnes-Taylor
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct
{
	char** tokens;
	int numTokens;
	int count;
} instruction;

void addToken(instruction* instr_ptr, char* tok);
void printTokens(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);
void parseCommand(instruction* instr_ptr);
char * expandVar(char* tok);
void expandPath(char* tok);
int inPath(const char* tok);
int isPath(const char* tok);
int isDir(const char* tok);
int isFile(const char* tok);
int fileExists(const char* tok);

int main()
{
	int exit = 0;

	char* token = NULL;
	char* temp = NULL;

	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;
	instr.count = 0;

	while(exit != 1){
		printf("%s@%s:%s> ", getenv("USER"), getenv("HOSTNAME"), getenv("PWD"));
		//printf("Please enter an instruction: ");
		//fgets(command, 100, stdin);
		// loop reads character sequences separated by whitespace
		do
		{
			scanf("%ms", &token);
			//scans for next token and allocates token var to size of scanned token
			temp = (char *)malloc((strlen(token) + 1) * sizeof(char));

			int i;
			int start = 0;
			for (i = 0; i < strlen(token); i++)
			{
				//pull out special characters and make them into a separate tokaaen in the instruction
				if (token[i] == '|' || token[i] == '>' || token[i] == '<' || token[i] == '&')
				{
					if (i - start > 0)
					{
						memcpy(temp, token + start, i - start);
						temp[i - start] = '\0';

						if (strcmp(temp, "exit") == 0)
							exit = 1;

						addToken(&instr, temp);
					}

					char specialChar[2];
					specialChar[0] = token[i];
					specialChar[1] = '\0';

					addToken(&instr, specialChar);

					start = i + 1;
				}
			}

			if (start < strlen(token))
			{
				memcpy(temp, token + start, strlen(token) - start);
				temp[i - start] = '\0';

				if (strcmp(temp, "exit") == 0)
					exit = 1;

				addToken(&instr, temp);
			}

			//free and reset variables
			free(token);
			free(temp);

			token = NULL;
			temp = NULL;
		} while ('\n' != getchar()); //until end of line is reached

		addNull(&instr);
		
		
		printTokens(&instr);
		clearInstruction(&instr);
	}

	return 0;
}

void parseCommand(instruction* instr_ptr)
{
	
}

char * expandVar(char* tok)
{
	// * Removes special character '$'
	// * Use getenv() to expand variable into a temp char ptr
	// * Takes in C string of purposed variable (e.g., $HOME, $PWD, etc.)
	// * Calculate size of 'temp' contents and use realloc() on passed in var 'tok' to give any 
	//	 needed space (include room for '\0')
	// * Use strcpy() or memcpy() to copy contents of 'temp' to 'tok'
	// * Result should be modifying 'tok' as such: '$HOME' --> '/home/kroot'
	// > Hayden >> I got this :^)

	// printf("%s: %s\n", "The tok to be expanded",tok);

	char *tmpStr;
	tmpStr = (char *) malloc((strlen(tok) + 1) * sizeof(char));

	if(tok[0] == '$')
	{
		//remove $
		memcpy(tmpStr, tok+1,sizeof(char)*strlen(tok));
	}

	char *var = getenv(tmpStr);

	free(tmpStr);

	// printf("%s: %s\n", "The tok",tok);
	// printf("%s: %s\n", "The var",var);

	return var;
}

void expandPath(char* tok)
{

}

//Looks for cmd stored in tok within directories returned by $PATH
//returns 1 on success 0 on failure
int inPath(const char* tok)
{
	char* fullPath;
	char* temp = "$PATH\0";
	char* path;
	expandPath(temp);

	path = strtok(temp, ":");
	while(path != NULL){
		fullPath = (char*)calloc(strlen(path) + strlen(tok)+ 1, sizeof(char));
		strcpy(fullPath, path);
		strcat(fullPath, tok);

		if(access(fullPath, X_OK) == 0){
			free(fullPath);
			printf("Success: command found\n");
			return 1;
		}
		else
			free(fullPath);
		path = strtok(temp, ":");
	}
	printf("Error: command not found\n");
	return 0;
}

int isPath(const char* tok)
{
	if (strstr(tok, "/") == NULL)
		return 0;
	
	return 1;
}

int isDir(const char* tok)
{
	struct stat* info = NULL;
	stat(tok, info);
	if (S_ISDIR(info->st_mode) == 0)
		return 0;

	return 1;
}

int isFile(const char* tok)
{

}

int fileExists(const char* tok)
{
	if( access( tok, F_OK ) != -1 ) 
	{
		return 1;
	} 
	else 
	{
		return 0;
	}
}

//reallocates instruction array to hold another token
//allocates for new token within instruction array
void addToken(instruction* instr_ptr, char* tok)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**) malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**) realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	//allocate char array for new token in new slot
	instr_ptr->tokens[instr_ptr->numTokens] = (char *)malloc((strlen(tok)+1) * sizeof(char));
	strcpy(instr_ptr->tokens[instr_ptr->numTokens], tok);

	instr_ptr->numTokens++;
}

void addNull(instruction* instr_ptr)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**)malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**)realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	instr_ptr->tokens[instr_ptr->numTokens] = (char*) NULL;
	instr_ptr->numTokens++;
}

void printTokens(instruction* instr_ptr)
{
	int i;
	printf("Tokens:\n");
	for (i = 0; i < instr_ptr->numTokens; i++) {
		if ((instr_ptr->tokens)[i] != NULL)
			printf("%s\n", (instr_ptr->tokens)[i]);
	}
}

void clearInstruction(instruction* instr_ptr)
{
	int i;
	for (i = 0; i < instr_ptr->numTokens; i++)
		free(instr_ptr->tokens[i]);

	free(instr_ptr->tokens);

	instr_ptr->tokens = NULL;
	instr_ptr->numTokens = 0;
}
