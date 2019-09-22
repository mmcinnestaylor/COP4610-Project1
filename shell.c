/*
 *  COP4610 - Assignment 1
 *  Authors: Keaun Moughari, Hayden Rogers, Marlan McInnes-Taylor
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/stat.h>

// regex pattern macros
#define PATH	"^[\/]*?[._]*?[a-zA-Z0-9_]+([\/_.-]*?[a-zA-Z0-9_]+)*"
#define ROOT 	"^\/+\.{1,2}*"
#define CMD		"^[a-zA-Z0-9_-]+$"

typedef struct
{
	char** tokens;
	int numTokens;
	int count;
	int error;
	int errCode;

} instruction;

void addToken(instruction* instr, char* tok);
void printTokens(instruction* instr);
void clearInstruction(instruction* instr);
void addNull(instruction* instr);

void runTests(instruction* instr);
void getCommand(instruction* instr);
void parseCommand(instruction* instr);
void printError(instruction* instr);
const char* getError(int e);

int expandPath(instruction* instr, int indx);
void cleanPath(char* tok);
void printWelcomeScreen();
int hasStr(instruction* instr, const char* str);
int isOp(const char* op);
int isPathOp(const char op);
int inPath(instruction *instr, int index);
int isPath(const char* tok);
int isRoot(const char* tok);
int isRel(const char* tok);
int isAbs(const char* tok);
int isDir(const char* tok);
int isFile(const char* tok);
int fileExists(const char* tok);
int lvlcnt(const char* tok);
int strcnt(const char* tok, int x);
int match(const char* tok, char* pattern);
char* expandVar(char* tok);
char* getPath();
void executeCommand(const char **cmd, const int size);

void b_exit();
void b_echo();
void b_cd();
void b_alias();
void b_unalias(); 

static cmdCount = 0;

int main()
{	
	int exit = 0;

	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;
	instr.error = -1;
	instr.errCode = -1;

	printWelcomeScreen();

	while(exit != 1)
	{
		getCommand(&instr);
		addNull(&instr);
		
		if (hasStr(&instr, "exit"))
			exit = 1;
		
		parseCommand(&instr);
		if (instr.error != -1)
			printError(&instr);

		printTokens(&instr);
		clearInstruction(&instr);
	}

	return 0;
}

/*
 *	Run Tests
 * 	>	instruction* 
 * 	:: 	void
 * 		
 * 	* iterate through intructions and run needed tests accordingly
 * 
 */
void runTests(instruction* instr)
{
	printf("%s:\n\n", "Tests");

	int i;
	for (i = 0; i < instr->numTokens; i++)
	{	
		if (instr->tokens[i] != NULL)
		{
			if (isPath(instr->tokens[i]))						// expandPath testing
			{
				printf("Path before: %s\n", instr->tokens[i]);
				expandPath(instr, i);
				printf("Path after: %s\n", instr->tokens[i]);
			}
		}
	}

	printf("\n");
}

void printError(instruction* instr)
{
	if (instr->error != -1)
	{
		if (instr->errCode == 4)
		{
			printf("bash: %s: '%s'\n",
				getError(instr->errCode),
				instr->tokens[instr->error] == NULL ? "newline" : instr->tokens[instr->error]);
		}
		else
			printf("bash: %s: %s\n", instr->tokens[instr->error], getError(instr->errCode));
	}
}

/*
 * get Command
 * 	>	instruction* 
 * 	:: 	void
 * 		
 * 	** CODE FROM GIVEN FILE PARSER_HELP.C
 *  * tokenize input for instruction struct
 * 
 */
void getCommand(instruction* instr)
{
		char* token = NULL;
		char* temp = NULL;
		
		printf("%s@%s : %s > ", getenv("USER"), getenv("HOSTNAME"), getenv("PWD"));
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
					
						addToken(instr, temp);
					}

					char specialChar[2];
					specialChar[0] = token[i];
					specialChar[1] = '\0';

					addToken(instr, specialChar);

					start = i + 1;
				}
			}

			if (start < strlen(token))
			{
				memcpy(temp, token + start, strlen(token) - start);
				temp[i - start] = '\0';

				addToken(instr, temp);
			}

			//free and reset variables
			free(token);
			free(temp);

			token = NULL;
			temp = NULL;

		} while ('\n' != getchar()); //until end of line is reached
}


/*
 *	Parse Command
 * 	>	instruction* 
 * 	:: 	void
 * 		
 * 
 */
void parseCommand(instruction* instr)
{
	int i, start = 0, end = 0, flag = 0;

	for (i = 0; i < instr->numTokens; i++)
	{
		end = i;
		if (isOp(instr->tokens[i]) || instr->tokens[i] == NULL)
		{
			if (instr->tokens[i] == NULL) {}
			else if (strcmp(instr->tokens[i], "|") == 0)
			{
				if (i == 0 || instr->tokens[i+1] == NULL)
				{
					instr->error = i;
					instr->errCode = 4;
					return;
				}

			}
			else if (strcmp(instr->tokens[i], "<") == 0)
			{
				if (instr->tokens[i+1] == NULL || isOp(instr->tokens[i+1]))
				{
					instr->error = i+1;
					instr->errCode = 4;
					return;
				}
			}
			else if (strcmp(instr->tokens[i], ">") == 0)
			{
				if (instr->tokens[i+1] == NULL || isOp(instr->tokens[i+1]))
				{
					instr->error = i+1;
					instr->errCode = 4;
					return;
				}
				else
				{
					flag = 1;
					for (i = i+1; i < instr->numTokens; i++)
					{
						end = i;
						if (instr->tokens[i] == NULL)
						{
							i--;
							break;
						}
						else if (match(instr->tokens[i], PATH))
						{
							if (!expandPath(instr, i))
								return;
							if (isDir(instr->tokens[i]))
							{
								instr->error = i;
								instr->errCode = 2;
								return;
							}
							
							if (!isFile(instr->tokens[i]))
							{
								FILE* file = fopen(instr->tokens[i], "w");
								if (!file)
								{
									instr->error = i;
									instr->errCode = 6;
									return;
								}
								else
									fclose(file);
							}
						}

						else
							break;	
					}

					continue;
				}
			}
			else if (strcmp(instr->tokens[i], "&") == 0)
			{
				if (i == 0)
				{
					if (instr->tokens[i+1] == NULL)
					{
						instr->error = i;
						instr->errCode = 4;
						return;
					}
					else
						continue;
				}
			} 

			switch (flag)
			{
				case 1:
				case 2:
				case 3:
				default:
					executeCommand(instr->tokens+start, end-start+1);
					break;
			}
			
			flag = 0;
			start = end + 1;
		}
		else if (i == start)		//assumes first tok is a cmd
		{
			if (strcmp(instr->tokens[i], "exit") == 0)
			{
				void b_exit();
			}
			else if (strcmp(instr->tokens[i], "cd") == 0)
			{
				void b_cd();
			}
			else if (strcmp(instr->tokens[i], "echo") == 0)
			{
				void b_echo();
			}
			else if (strcmp(instr->tokens[i], "alias") == 0)
			{
				void b_alias();	
			}
			else if (strcmp(instr->tokens[i], "unalias") == 0)
			{
				void b_unalias(); 
			}
			else if (strcnt(instr->tokens[i], '/') == 0 && !match(instr->tokens[i], "^[\.]{1,2}"))
			{
				if (!inPath(instr, i))
					return;
			}
			else	// check for errors in case first tok of cmd is not valid
			{
				if (!expandPath(instr, i))
					return;
				
				if (isDir(instr->tokens[i]))
				{
					instr->error = i;
					instr->errCode = 2;
					return;
				}
				else if (!isFile(instr->tokens[i]))
				{
					instr->error = i;
					instr->errCode = 0;
					return;
				}
				else if (access(instr->tokens[i], X_OK) != 0)
				{
					instr->error = i;
					instr->errCode = 5;
					return;	
				}
					
					
			}
		}
		else if (match(instr->tokens[i], PATH) || isPath(instr->tokens[i]))	//process path args
		{
			if (!expandPath(instr, i))
					return;
		}
	}
}

/*
 *	has String
 * 	>	instruction* 
 * 	>	const char*
 * 	:: 	int
 * 	
 * 	* iterate through instr's tokens searching for 'str' (string must fully match)
 * 	* if found, return 1
 * 	* otherwise, return 0
 * 
 */
int hasStr(instruction* instr, const char* str)
{
	int i;
	for (i = 0; i < instr->numTokens; i++)
	{	
		if (instr->tokens[i] != NULL)
		{
			if (strcmp(instr->tokens[i], str) == 0)
				return 1;
		}
	}

	return 0;
}

/*
 *	Expand Variable
 * 	>	char* 
 * 	:: 	char*
 * 
 * 	* Removes special character '$'
 *	* Use getenv() to expand variable into a temp char ptr
 *	* Takes in C string of purposed variable (e.g., $HOME, $PWD, etc.)
 *	* Calculate size of 'temp' contents and use realloc() on passed in var 'tok' to give any 
 *		 needed space (include room for '\0')
 *	* Use strcpy() or memcpy() to copy contents of 'temp' to 'tok'
 * 	* Result should be modifying 'tok' as such: '$HOME' --> '/home/kroot'
 * 
 */
char* expandVar(char* tok)
{
	if (tok == NULL)
		return NULL;
	
	char *tmpStr = NULL;
	if(tok[0] == '$')
	{
		tmpStr = (char *) calloc(strlen(tok) + 1, sizeof(char)); 
		//remove $
		memcpy(tmpStr, tok+1, sizeof(char)*strlen(tok));
	}
	else
	{
		return NULL;
	}

	char *var = getenv(tmpStr);
	if (var == NULL)
		return NULL;
	char *tmpVar = (char *) calloc(strlen(var) + 1, sizeof(char));
	strcpy(tmpVar, var);

	if (tmpStr != NULL)
		free(tmpStr);

	return tmpVar;
}

/*
 *	Clean Path
 * 	>	char* 
 * 	:: 	void
 * 
 * 	* cleans the path sent in from redundant forward slashes
 *  * also removes trailing slashes at the end of the path 
 */
void cleanPath(char* tok)
{
	if (!isPath(tok))
		return;

	char* temp = (char*) calloc(strlen(tok) + 1, sizeof(char));
	int i, start, repeat = 0, size = strlen(tok);
	for (i = 0; i < size; i++)
	{
		if (tok[i] == '/')
		{
			start = i;
			while (tok[i+1] == '/')
			{
				repeat++;
				i++;
			}

			if (repeat > 0)
			{
				strcpy(temp, tok + i+1);
				tok[start+1] = '\0';
				strcat(tok, temp);
				i = start + 1;
				repeat = 0;
				size = strlen(tok);
			}
		}
	}
	
	if (size > 1 && tok[size-1] == '/')
		tok[size-1] = '\0';
	tok = (char*) realloc(tok, (strlen(tok) + 1) * sizeof(char));
	free(temp);	
}

/*
 *	Expand Path
 * 	>	char* 
 * 	:: 	void
 * 
 * 	* transforms token by tokenizing it based on type of path and
 * 		what directory operands are present
 * 	* function is atomic, if there is an error, it the tok will be flagged
 * 		but 'tok' will not be changed
 *  * if successful, 'tok' will reflect absolute path
 */
int expandPath(instruction* instr, int indx)
{	
	if (instr->tokens[indx] == NULL || isRoot(instr->tokens[indx]))
		return 1;

	char* pwd = getPath(); 
	if (pwd == NULL)
		printf("%s", "bash: Error retrieving current path");

	char** temp = (char**) calloc(lvlcnt(instr->tokens[indx]), sizeof(char*)); 	
	char* tmpTok = (char*) calloc(strlen(instr->tokens[indx]) + 1, sizeof(char));
	strcpy(tmpTok, instr->tokens[indx]); 
	char* part = strtok(tmpTok, "/");		//tokenize by '/' delimiter
	int i = -1;
	while (part != NULL)
	{
		i++;
		if (i == 0) 					
		{	
			if (isRel(instr->tokens[indx])) 	// fill temp[0] with $PWD if tok is relative
			{
				temp[i] = (char*) calloc(strlen(pwd) + 1, sizeof(char));
				strcpy(temp[i], pwd);
				i++;
			}
			else if (match(instr->tokens[indx], ROOT))  // make temp[0] '/' if root
			{
				temp[i] = (char*) calloc(2, sizeof(char));
				strcpy(temp[i], "/");
				i++;
			}
		}

		temp[i] = (char*) calloc(strlen(part) + 1, sizeof(char)); 

		strcpy(temp[i], part);
		part = strtok(NULL, "/");
	}

	free(tmpTok);
	char* expand = NULL;
	int count = i+1, size = 0;
	for (i = 0; i < count; i++)
	{
		if (strcmp(temp[i], "~") == 0)
		{
			if (i != 0)
			{
				instr->error = indx;
				instr->errCode = 0;
				i = count;
			}
			else 
			{
				expand = getenv("HOME");
				temp[i] = (char*) realloc(temp[i], (strlen(expand) + 1) * sizeof(char));
				strcpy(temp[i], expand);
			}
		}
		else if (strcmp(temp[i], ".") == 0)
		{

		}
		else if (strcmp(temp[i], "..") == 0)
		{	
			if (isRoot(temp[0]))
			{
				instr->error = indx;
				instr->errCode = 0;
				i = count;
			}
			else
			{
				char *tmp = NULL;
				size = strlen(temp[0]) + 1;
				expand = (char*) calloc(size, sizeof(char));
				strcpy(expand, temp[0]);
				tmp = strrchr(expand, '/');
				*tmp = '\0';
				if (strcnt(expand, '/') == 0)
					strcpy(expand, "/");

				if (!isDir(expand))
				{	
					if (isFile(expand))
					{
						instr->error = indx;
						instr->errCode = 1;
						i = count;
					}
					else
					{
						instr->error = indx;
						instr->errCode = 0;
						i = count;
					}
				}
				else
				{
					temp[0] = (char*) realloc(temp[0], (strlen(expand) + 1) * sizeof(char));
					strcpy(temp[0], expand);
				}

				free(expand);
				expand = NULL;
			}
		}
		else if (strncmp(temp[i], "$", 1) == 0)
		{
			expand = expandVar(temp[i]);
			if (expand != NULL)
			{
				temp[i] = (char*) realloc(temp[i], (strlen(expand) + 1) * sizeof(char));
				strcpy(temp[i], expand);
				if (strcmp(temp[i], temp[0]) != 0)		// protect against env var(x2) in 
				{											// case of relative path
					size = strlen(temp[0]) + strlen(temp[i]) + 1;
					expand = (char*) calloc(size, sizeof(char));
					strcpy(expand, temp[0]);
					strcat(expand, temp[i]);
					
					temp[0] = (char*) realloc(temp[0], (strlen(expand) + 1) * sizeof(char));
					strcpy(temp[0], expand);
					
					free(expand);
					expand = NULL;
				}
			}

		}
		else
		{
			if (i == 0)
				continue;
			else 
			{	
				size = strlen(temp[0]) + strlen(temp[i]) + 1;
				
				expand = (char*) calloc(size, sizeof(char));
				strcpy(expand, temp[0]);
				if (!isRoot(temp[0]))
					strcat(expand, "/");
				strcat(expand, temp[i]);
				
				temp[0] = (char*) realloc(temp[0], (strlen(expand) + 1) * sizeof(char));
				strcpy(temp[0], expand);

				free(expand);
				expand = NULL;
			}
		}

		if (i+1 == count)
			size = (strlen(temp[0]) + 1);
	}

	if (instr->error == -1) 
	{
		instr->tokens[indx] = (char*) realloc(instr->tokens[indx], size * sizeof(char));
		strcpy(instr->tokens[indx], temp[0]);
	}
	
	for (i = 0; i < count; i++)
		free(temp[i]);
	free(temp);
	free(pwd);

	if (instr->error == -1)
		return 1;
	else
		return 0;
}

//Looks for cmd stored in tok within directories returned by $PATH
//returns 1 on success 0 on failure
int inPath(instruction *instr, int index /*, char* tok*/)
{
	char *fullPath = NULL;
	char *temp = expandVar("$PATH\0");
	char *path = NULL;

	path = strtok(temp, ":");

	while (path != NULL)
	{
		fullPath = (char *)calloc(strlen(path) + strlen((instr->tokens)[index] /*tok*/) + 2, sizeof(char));
		strcpy(fullPath, path);
		strcat(fullPath, "/");
		strcat(fullPath, (instr->tokens)[index]);

		if (isFile(fullPath))
		{
			if (access(fullPath, X_OK) == 0)
			{
				free(temp);
				free((instr->tokens)[index]);
				(instr->tokens)[index] = fullPath;
				return 1;
			}
		}
		else
			free(fullPath);
		path = strtok(NULL, ":");
	}
	free(temp);
	instr->error = index;
	instr->errCode = 0;
	return 0;
}

/*
 *	is Root
 * 	>	const char* 
 * 	:: 	int
 * 
 * 	* checks if token is root ("/")
 * 	* if it is, returns 1
 *  * otherwise, returns 0
 */
int isRoot(const char* tok)
{
	if (tok != NULL)
		if (tok[0] == '/' && tok[1] == '\0')
			return 1;

	return 0;
}


/*
 *	is Path
 * 	>	const char* 
 * 	:: 	int
 * 
 * 	* checks if token contains a forward slash
 * 	* if it does, assumed to be a path; returns 1
 *  * otherwise, returns 0
 */
int isPath(const char* tok)
{	
	if (tok != NULL)
		if (match(tok, "^(/?[^/&|<>-]*)+/?$"))
			return 1;

	return 0;
}


/*
 *	is Relative
 * 	>	const char* 
 * 	:: 	int
 * 
 * 	* checks if path is relative by checking first char
 * 	* if relative, returns 1
 *  * otherwise, returns 0
 */
int isRel(const char* tok)
{
	if (isPath(tok))
		if (tok[0] != '/' && tok[0] != '~' && tok[0] != '$')
			return 1;
	
	if (tok[0] == '.')
		return 1;
	
	return 0;
}

/*
 *	is Absolute
 * 	>	const char* 
 * 	:: 	int
 * 
 * 	* checks if path is absolute by checking first char
 * 	* if absolute, returns 1
 *  * otherwise, returns 0
 */
int isAbs(const char* tok)
{
	if (isPath(tok))
		if (!isRel(tok))
			return 1;
	
	return 0;
}


/*
 *	is Directory
 * 	>	const char* 
 * 	:: 	int
 * 
 * 	* checks if token is a directory using stat.h
 * 	* if it is, returns 1
 *  * otherwise, returns 0
 */
int isDir(const char* tok)
{
	struct stat info;
	if (stat(tok, &info) == 0 && S_ISDIR(info.st_mode))
		return 1;

	return 0;
}

/*
 *	is File
 * 	>	const char* 
 * 	:: 	int
 * 
 * 	* check if token is a file using stat.h
 * 	* if it is, returns 1
 *  * otherwise, returns 0
 * 
 */
int isFile(const char* tok)
{
	struct stat info;
	if (stat(tok, &info) == 0 && S_ISREG(info.st_mode))
		return 1;

	return 0;
}
/*
 *	file Exists
 * 	>	const char* 
 * 	:: 	int
 * 
 * 	* checks if token contains a forward slash
 * 	* if it does, assumed to be a path; returns 1
 *  * otherwise, returns 0
 * 
 */
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

/*
 *	is Path Operator
 * 	>	const char
 * 	:: 	int
 * 
 * 	* checks if token is a path operator
 * 	* if it is, returns 1
 *  * otherwise, returns 0
 * 
 */
int isPathOp(const char op)
{
	if (op == '.')			return 1;
	else if (op == '$')		return 1;
	else if (op == '~')		return 1;
	else 					return 0;
}

/*
 *	is Operator
 * 	>	const char
 * 	:: 	int
 * 
 * 	* checks if token is a cmd operator
 * 	* if it is, returns 1
 *  * otherwise, returns 0
 * 
 */
int isOp(const char* op)
{
	if (op == NULL)					return 0;
	else if (strcmp(op, "|") == 0)	return 1;
	else if (strcmp(op, ">") == 0) 	return 1;
	else if (strcmp(op, "<") == 0)	return 1;
	else if (strcmp(op, "&") == 0)	return 1;
	else 							return 0;
}

/*
 *  String Count
 * 	>	const char* 
 * 	>	int
 * 	:: 	int
 * 
 * 	* run through tok and count every occurance of char x
 * 		which is represented as an int  
 * 	* returns count
 * 
 */
int strcnt(const char* tok, int x)
{	
	int i, count = 0;
	for (i = 0; i < strlen(tok); i++)
	{
		if (tok[i] == x)
			count++;
	}

	return count;
}

/*
 *  Level Count
 * 	>	const char* 
 * 	:: 	int
 * 
 * 	* run through tok and counts how many 'levels' or spaces
 * 		will be needed for expandPath's double char pointer allocation
 * 
 */
int lvlcnt(const char* tok)
{
	int i, count = 0;
	if (isRel(tok))
		count++;

	for (i = 0; i < strlen(tok); i++)
	{	
		switch (tok[i])
		{
			case '~':
				count++;
				break;
			case '.':
				if (tok[i+1] == '.')
				{
					count++;
					
					if (tok[i+1] != '\0')
						i++;
				}
				else if (tok[i+1] == '/' || tok[i+1] == '\0')
					count++;
				break;
			case '$':
				count++;
				while (i < strlen(tok) && tok[i+1] != '/' && !isPathOp(tok[i+1]))
					i++;
				break;
			default:
				if (tok[i] == '/')
				{
					if (i == 0)
						count++;
				}
				else
				{
					count++;
					while (i < strlen(tok) && tok[i+1] != '/' && match(tok + i+1, PATH))
						i++;
				}
				break;
		}
	}

	return count;
}

/*
 *	match
 * 	>	const char*
 *  > 	char*  
 * 	:: 	int
 * 
 * 	* uses regex.h lib for regular expressions
 * 	* analyzes tok using given patterns
 * 	* if there is a match, return 1
 *  * otherwise, returns 0
 * 
 */
int match(const char* tok, char* pattern)
{
	int stat;
    regex_t re;

    if (regcomp(&re, pattern, REG_EXTENDED|REG_ICASE|REG_NOSUB) != 0)
    	return 0;

    stat = regexec(&re, tok, (size_t) 0, NULL, 0);
	regfree(&re);
    
	if (stat != 0)
        return 0;

	return 1;
}

/*
 *	get Error
	>	int 
 * 	:: 	const char*
 * 
 * 	* return error (string) corresponding to error code passed in
 * 
 */
const char* getError(int e)
{
	if (e == 0)			return "No such file or directory";
	else if (e == 1)	return "Not a directory";
	else if (e == 2) 	return "Is a directory";
	else if (e == 3)	return "Command not found";
	else if (e == 4)	return "Syntax error near unexpected token";
	else if (e == 5)	return "Permission denied";
	else if (e == 6)	return "Error creating file";
	else				return NULL;
}

/*
 *	get Path 
 * 	:: 	char*
 * 
 * 	** USER MUST ALWAYS FREE MEMORY ALLOCATED BY THIS FUNCTION
 * 	* allocates space for $PWD string and returns
 * 		it as a char ptr
 */
char* getPath()
{
	char* pwd = NULL, *temp = getenv("PWD");
	if (temp != NULL) 
	{
		pwd = (char*) calloc(strlen(temp) + 1, sizeof(char));
		strcpy(pwd, temp);
	}

	return pwd;
}

void executeCommand(const char **cmd, const int size)
{
	int status, i;
	pid_t pid;
	//copy instructions into new array
	
	char** argv = (char**) calloc(size, sizeof(char*));
	for (i = 0; i < size; i++)
	{
		if (i+1 == size)
		{
			argv[i] = (char*) NULL;
		}
		else
		{
			argv[i] = (char*) calloc(strlen(cmd[i]) + 1, sizeof(char));
			strcpy(argv[i], cmd[i]);
			printf("Arg %d: %s\n", i, argv[i]);
		}
	}

	if ((pid = fork()) == -1)
	{
		//Error
		exit(1);
	}
	else if (pid == 0)
	{
		//Child
		//printf("The command to be executed is: %s", cmd[0]);
		execv(argv[0], argv);
		//printf("Error running command\n");
		//printf(“Problem executing %s \n”, cmd[0]);
		exit(1);
	}
	else
	{
		//Parent
		waitpid(pid, &status, 0);
	}

	for (i = 0; i < size; i++)
		free(argv[i]);
	free(argv);
}

void b_exit()
{

}

void b_echo()
{

}

void b_cd()
{

}

void b_alias()
{

}

void b_unalias()
{

} 

void printWelcomeScreen()
{
	printf("\n%s\n", " ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ");
	printf("%s\n", "       __              ___    ___                 ");
	printf("%s\n", "      /\\ \\            /\\_ \\  /\\_ \\                ");
	printf("%s\n", "  ____\\ \\ \\___      __\\//\\ \\ \\//\\ \\          ___   ");
	printf("%s\n", " /',__\\\\ \\  _ `\\  /'__`\\\\ \\ \\  \\ \\ \\       /'___\\ ");
	printf("%s\n", "/\\__, `\\\\ \\ \\ \\ \\/\\  __/ \\_\\ \\_ \\_\\ \\_  __/\\ \\__/ ");
	printf("%s\n", "\\/\\____/ \\ \\_\\ \\_\\ \\____\\/\\____\\/\\____\\/\\_\\ \\____\\");
	printf("%s\n\n", " \\/___/   \\/_/\\/_/\\/____/\\/____/\\/____/\\/_/\\/____/");
}



//BEGINNING OF GIVEN PARSER CODE
//reallocates instruction array to hold another token
//allocates for new token within instruction array
void addToken(instruction* instr, char* tok)
{
	//extend token array to accomodate an additional token
	if (instr->numTokens == 0)
		instr->tokens = (char**) malloc(sizeof(char*));
	else
		instr->tokens = (char**) realloc(instr->tokens, (instr->numTokens+1) * sizeof(char*));

	// check if tok is a path, if so clean it of excess slashes
	if (isPath(tok))
		cleanPath(tok);

	//allocate char array for new token in new slot
	instr->tokens[instr->numTokens] = (char *)malloc((strlen(tok)+1) * sizeof(char));
	strcpy(instr->tokens[instr->numTokens], tok);

	instr->numTokens++;
}

void addNull(instruction* instr)
{
	//extend token array to accomodate an additional token
	if (instr->numTokens == 0)
		instr->tokens = (char**)malloc(sizeof(char*));
	else
		instr->tokens = (char**)realloc(instr->tokens, (instr->numTokens+1) * sizeof(char*));

	instr->tokens[instr->numTokens] = (char*) NULL;
	instr->numTokens++;
}

void printTokens(instruction* instr)
{
	int i;
	printf("Tokens:\n");
	for (i = 0; i < instr->numTokens; i++) {
		if ((instr->tokens)[i] != NULL)
			printf("%s\n", (instr->tokens)[i]);
	}
}

void clearInstruction(instruction* instr)
{
	int i;
	for (i = 0; i < instr->numTokens; i++)
		free(instr->tokens[i]);

	free(instr->tokens);

	instr->tokens = NULL;
	instr->numTokens = 0;
	instr->error = -1;
	instr->errCode = -1;
}