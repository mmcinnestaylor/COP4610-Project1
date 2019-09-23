/*
 *  COP4610 - Assignment 1
 *  Authors: Keaun Moughari, Hayden Rogers, Marlan McInnes-Taylor
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
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

typedef struct
{
	char* cmd;
	char* cmdAlias;

} pair;

typedef struct
{
	pair arr[10];
	int arrSize;
	int maxSize;

} alias;

static alias aliases;
static int instrCount = 0;
static int myExit = 0;

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
void cleanPath(instruction* instr, int indx);
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
void executeRedirection(const char** cmd, const int flag);
void clearAliases();
int getIndex(const char* tok);
void layPipe(instruction* instr_ptr);

void b_exit(int instrCount);
void b_echo(const char** cmd, const int size); 
void b_cd(const char* path); // if folder is in CWD must append ./ or segfault
void b_alias(const char** cmd);
void b_unalias(const char** cmd); 

int main()
{	
	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;
	instr.error = -1;
	instr.errCode = -1;

	aliases.maxSize = 10;
	aliases.arrSize = 0;

	printWelcomeScreen();

	while(myExit == 0)
	{
		getCommand(&instr);
		addNull(&instr);
		
		parseCommand(&instr);
		if (instr.error != -1)
			printError(&instr);

		printTokens(&instr);
		clearInstruction(&instr);
	}
	
	return 0;
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
			//if (getIndex(token) != -1)
			//{
			//	int i = getIndex(token);
			//	free(token);
			//	token = (char*) calloc(strlen(aliases.arr[i].cmd) + 1, sizeof(char));
			//	strcpy(token, aliases.arr[i].cmd);
			//}
			
			
			temp = (char *)malloc((strlen(token) + 1) * sizeof(char));

			int i;
			int start = 0;
			for (i = 0; i < strlen(token); i++)
			{
				//pull out special characters and make them into a separate tokaaen in the instruction
				if (token[i] == '|' || token[i] == '>' || token[i] == '<' || token[i] == '&' || 
					token[i] == '=' || token[i] == '\'')
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
					
					if (token[i] != '\'')
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
	int i, start = 0, end = 0, flag = 0, io = 0;

	for (i = 0; i < instr->numTokens; i++)
	{
		end = i;
		if (isOp(instr->tokens[i]) || instr->tokens[i] == NULL)
		{
			if (instr->tokens[i] == NULL) {}
			else if (strcmp(instr->tokens[i], "|") == 0)
			{
				flag = 2;
				if (i == start || instr->tokens[i+1] == NULL)
				{
					instr->error = i;
					instr->errCode = 4;
					return;
				}
				else
				{
					for (i = i+1; i < instr->numTokens; i++)
					{
						end = i;
						if (instr->tokens[i] == NULL)
						{
							i--;
							break;
						}
						else if (strcmp(instr->tokens[i-1], "|") == 0)
						{
							if (!inPath(instr, i))
								return;
						}
						else if (match(instr->tokens[i], "^-{1,2}+")) {}
						else if (match(instr->tokens[i], PATH))
						{
							if (!expandPath(instr, i))
								return;
							if (!isDir(instr->tokens[i]) || !isFile(instr->tokens[i]))
							{
								instr->error = i;
								instr->errCode = 0;
								return;
							}
						}
						else
							break;	
					}

					continue;
				}
			}
			else if (strcmp(instr->tokens[i], "<") == 0)
			{
				flag = 1;
				if (instr->tokens[i+1] == NULL || isOp(instr->tokens[i+1]))
				{
					instr->error = i+1;
					instr->errCode = 4;
					return;
				}
				else
				{
					io = 1;
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
								if (io == 3)
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
								else 
								{
									instr->error = i;
									instr->errCode = 0;
									return;
								}
							}
						}
						else if (strcmp(instr->tokens[i], ">") == 0)
							io = 3;
						else
							break;	
					}

					continue;
				}	
			}
			else if (strcmp(instr->tokens[i], ">") == 0)
			{
				flag = 1;
				if (instr->tokens[i+1] == NULL || isOp(instr->tokens[i+1]))
				{
					instr->error = i+1;
					instr->errCode = 4;
					return;
				}
				else
				{
					io = 2;
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
						else if (strcmp(instr->tokens[i], "<") == 0)
						{
							io = 4;
							break;
						}
						else
							break;	
					}

					if (strcmp(instr->tokens[0], "echo") == 0)
						io = 5;
	
					continue;
				}
			}
			else if (strcmp(instr->tokens[i], "&") == 0)
			{	
				// lmaooooooooooooooooooo
				flag = 4;
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
			
			int j;
			switch (flag)
			{
				case 1:
					executeRedirection(instr->tokens+start, io);
					break;
				case 2:
					layPipe(instr->tokens);
				case 3:
					if (i == 1)
					{
						b_cd(getenv("HOME"));
						return;
					}	
					else
					{
						b_cd(instr->tokens[i]);
						return;
					}
				default:
					executeCommand(instr->tokens+start, end-start+1);
					break;
			}
			
			flag = 0;
			io = 0;
			start = end + 1;
		}

		// assumes first tok is a cmd (start should always be)
		else if (i == start)		
		{
			if (strcmp(instr->tokens[i], "exit") == 0)
			{
				b_exit(instrCount);
			}
			else if (strcmp(instr->tokens[i], "cd") == 0)
			{
				flag = 3;
			}
			else if (strcmp(instr->tokens[i], "echo") == 0)
			{
				// find where echo args end
				for (i = i+1; i < instr->numTokens; i++)
				{
					end = i;
					if (isOp(instr->tokens[i]) || instr->tokens[i] == NULL)
					{
						i--;
						break;
					}
					else if (match(instr->tokens[i], "^[$]+[a-zA-Z]+$"))
					{
						if (!expandPath(instr, i))
							printf("bash: %s: not valid env var\n", instr->tokens[i]);
						instr->error = -1;
					}
				}

				if (instr->tokens[i+1] == NULL)
				{
					b_echo(instr->tokens+start, end-start);
					start = end;
				}
			}
			else if (strcmp(instr->tokens[i], "alias") == 0)
			{	
				b_alias(instr->tokens);
				return;	
			}
			else if (strcmp(instr->tokens[i], "unalias") == 0)
			{
				b_unalias(instr->tokens);
				return;
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
			if (strcnt(instr->tokens[i], '/') > 0 && !expandPath(instr, i))
			{
				return;
			
				if (!(isDir(instr->tokens[i]) || isFile(instr->tokens[i])))
				{
					instr->error = i;
					instr->errCode = 0;
					return;
				}
			}
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
void cleanPath(instruction* instr, int indx)
{
	if (!isPath(instr->tokens[indx]))
		return;
	char* tok = (char*) calloc(strlen(instr->tokens[indx]) + 1, sizeof(char));
	strcpy(tok, instr->tokens[indx]);
	char* temp = (char*) calloc(strlen(instr->tokens[indx]) + 1, sizeof(char));
	int i, start, repeat = 0, size = strlen(instr->tokens[indx]);
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
	
	
	instr->tokens[indx] = (char*) realloc(instr->tokens[indx], (strlen(tok) + 1) * sizeof(char));
	strcpy(instr->tokens[indx], tok);
	free(tok);
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

	cleanPath(instr, indx);
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
			else
				instr->error = i;
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
	instr->errCode = 3;
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
		execv(argv[0], argv);
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
	instrCount++;
}

void executeRedirection(const char** cmd, const int flag){
	int i = 0, j = 0, fd = 0, fd2 = 0, inIndex = 0, outIndex = 0, status;
	char operatorIn[] = "<", operatorOut[] = ">";
	char** argv = NULL;
	pid_t pid;
	
	//Input
	if(flag == 1){
		while(cmd[i] != NULL){
			if(strcmp(cmd[i], operatorIn) == 0)
				break;
			else
				i++;
		}

		argv = (char**) calloc(i + 1, sizeof(char*));
		for(j = 0; j < i; j++)
			argv[j] = cmd[j];
		argv[i] = NULL;
		
		fd = open(cmd[i + 1], O_RDONLY);
		if(fd != -1 && (pid = fork()) == 0){
			//close(STDIN_FILENO);
			dup2(fd, STDIN_FILENO);
			close(fd);
			execv(argv[0], argv);
		}
		else{
			waitpid(pid, &status, 0);
			close(fd);
		}
	}
	//Output
	else if(flag == 2){
		while(cmd[i] != NULL){
			if(strcmp(cmd[i], operatorOut) == 0)
				break;
			else
				i++;
		}
		argv = (char**) calloc(i + 1, sizeof(char*));
		for(j = 0; j < i; j++)
			argv[j] = cmd[j];
		argv[i] = NULL;

		fd = open(cmd[i + 1], O_WRONLY | O_TRUNC);
		if(fd != -1 && (pid = fork()) == 0){
			close(STDOUT_FILENO);
			dup(fd);
			close(fd);
			execv(argv[0], argv);
		}
		else{
			waitpid(pid, &status, 0);
			close(fd);
		}
	}
	//Input + Output
	else if(flag == 3){
		while(cmd[i] != NULL){
			if(strcmp(cmd[i], operatorIn) == 0){
				inIndex = i;
				break;
			}
			else
				i++;
		}
		while(cmd[i] != NULL){
			if(strcmp(cmd[i], operatorOut) == 0){
				outIndex = i;
				break;
			}
			else
				i++;
		}

		argv = (char**) calloc(inIndex + 1, sizeof(char*));
		for(j = 0; j < inIndex; j++)
			argv[j] = cmd[j];
		argv[inIndex] = NULL;

		fd = open(cmd[inIndex + 1], O_RDONLY);
		fd2 = open(cmd[outIndex + 1], O_WRONLY | O_TRUNC);
		if(fd != -1 && fd2 != -1 && (pid = fork()) == 0){
			close(STDIN_FILENO);
			dup(fd);
			close(STDOUT_FILENO);
			dup(fd2);
			close(fd);
			close(fd2);
			execv(argv[0], argv);
		}
		else{
			waitpid(pid, &status, 0);
			close(fd);
			close(fd2);
		}
	}	
	//Output + Input
	else if(flag == 4){
		while(cmd[i] != NULL){
			if(strcmp(cmd[i], operatorOut) == 0){
				outIndex = i;
				break;
			}
			else
				i++;
		}
		while(cmd[i] != NULL){
			if(strcmp(cmd[i], operatorIn) == 0){
				inIndex = i;
				break;
			}
			else
				i++;
		}

		argv = (char**) calloc(outIndex + 1, sizeof(char*));
		for(j = 0; j < outIndex; j++)
			argv[j] = cmd[j];
		argv[outIndex] = NULL;

		fd = open(cmd[inIndex + 1], O_RDONLY);
		fd2 = open(cmd[outIndex + 1], O_WRONLY | O_TRUNC);
		if(fd != -1 && fd2 != -1 && (pid = fork()) == 0){
			close(STDIN_FILENO);
			dup(fd);
			close(STDOUT_FILENO);
			dup(fd2);
			close(fd);
			close(fd2);
			execv(argv[0], argv);
		}
		else{
			waitpid(pid, &status, 0);
			close(fd);
			close(fd2);
		}
	}
	//echoing to a file
	else if(flag == 5){
		while(cmd[i] != NULL){
			if(strcmp(cmd[i], operatorOut) == 0)
				break;
			else
				i++;
		}

		fd = open(cmd[i + 1], O_WRONLY | O_TRUNC);
		int temp = dup(STDOUT_FILENO);
		if(fd != -1){
			close(STDOUT_FILENO);
			dup(fd);
			close(fd);
			b_echo(cmd, i);
			close(1);
			dup(temp);
		}
		
	}
	free(argv);
	instrCount++;
}

void clearAliases(){
	int i = 0;
	for(i; i < aliases.arrSize; i++){
		free((aliases.arr[i]).cmdAlias);
		free((aliases.arr[i]).cmd);
	}
}

int getIndex(const char* tok){
	int i = 0;
	for(i; i < aliases.arrSize; i++){
		if(strcmp(tok, (aliases.arr[i]).cmdAlias) == 0)
			return i;
	}
	return -1;
}


void b_exit(int instrCount)
{
	clearAliases();
	printf("Exiting...\n");
	printf("\tCommands executed: %d\n", instrCount);
	myExit = 1;
}

void b_echo(const char** cmd, const int size)
{
	int i=0;
	for(i = 1; i < size; i++)
		printf("%s ", cmd[i]);
	printf("\n");
}

void b_cd(const char* path)
{
	char* temp = (char*) calloc(strlen(path) + 1, sizeof(char));
	strcpy(temp, path);

	if (strcmp(path, "~"))
	{
		free(temp);
		temp = expandVar("$HOME\0");
	}

	if(chdir(temp) == 0)
		setenv("PWD", temp, 1);
}

void b_alias(const char** cmd)
{
	int i = 0, length = 0, opIndex = 0;
	while(cmd[i] != NULL){
		if(strcmp(cmd[i], "=") == 0)
			break;
		else
			i++;
	}

	(aliases.arr[aliases.arrSize]).cmdAlias = (char*) calloc(strlen(cmd[i-1]) + 1, sizeof(char));
	strcpy((aliases.arr[aliases.arrSize]).cmdAlias, cmd[i-1]);

	opIndex = i;
	i++;
	while(cmd[i] != NULL){
		length += strlen(cmd[i] + 1);
		i++;
	}

	(aliases.arr[aliases.arrSize]).cmd = (char*) calloc(length + 1, sizeof(char));
	i = opIndex + 1;
	strcpy((aliases.arr[aliases.arrSize]).cmd, cmd[i]);
	i++;
	while(cmd[i] != NULL){
		strcat((aliases.arr[aliases.arrSize]).cmd, " ");
		strcat((aliases.arr[aliases.arrSize]).cmd, cmd[i]);
		i++;
	}
	aliases.arrSize++;
}

void b_unalias(const char** cmd)
{
	int i;
	for(i = 0; i < aliases.maxSize; i++){
		if(strcmp((aliases.arr[i]).cmdAlias, cmd[1]) == 0){
			free((aliases.arr[i]).cmdAlias);
			free((aliases.arr[i]).cmd);
			break;
		}
	}

	for(i; i < aliases.arrSize; i++){
			aliases.arr[i] = aliases.arr[i + 1];
	}
	aliases.arrSize--;
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

// this function laid pipe in me if you catch my drift
void layPipe(instruction* instr_ptr) 
{

	int i;
	int numPipes = 0;
	int size = instr_ptr->numTokens;
	int cmd_1_size = 0;
	int cmd_2_size = 0;
	int cmd_3_size = 0;
	int pipeEncountered = 0;

	int pipeOneIndex = 0;
	int pipeTwoIndex = 0;


	// get num pipes
	for (i = 0; i < instr_ptr->numTokens; i++) 
	{ 
		if ( (instr_ptr->tokens)[i] != NULL) 
		{	 
			if( ( strcmp((instr_ptr->tokens)[i],"|") == 0) )
			{ 
				numPipes+=1;

				// if pipe has been encountered,
				// store location of pipe
				if (numPipes > 0)
				{
					if (pipeEncountered == 0)
						pipeOneIndex = i;
					if (pipeEncountered == 1)
						pipeTwoIndex = i;

					pipeEncountered+=1;
				}
			}

		} 
		// else
		// 	printf("%s\n", "null encountered");
	} 

	if (numPipes > 2)
	{
		printf("Too many pipes... Maximum of [2] pipes supported.\n");
		return;
	}

	if (numPipes == 0)
	{
		printf("No pipes encontered... returning...\n");
		return;
	}

	// allocate memory	
	char** cmd1 = (char**) calloc(size, sizeof(char*));
	char** cmd2 = (char**) calloc(size, sizeof(char*));
	char** cmd3 = (char**) calloc(size, sizeof(char*));

	// handle cammand copies for one pipe
	if (numPipes == 1)
	{
		// gotta populate cmd1 
		for (i = 0; i <= pipeOneIndex; i++)
		{
			if (i == pipeOneIndex)
			{
				printf("%s\n", "finishing things off with a null");
				cmd1[i] = (char*) NULL;
			}
			else
			{
				//printf("%s\n", "adding shit to cmd one");
				cmd1[i] = (char*) calloc(strlen(instr_ptr->tokens[i]) + 1, sizeof(char));
				strcpy(cmd1[i], instr_ptr->tokens[i]);
				printf("cmd1 %d: %s\n", i, cmd1[i]);
			}
			cmd_1_size+=1;
		}

		// gotta populate cmd2
		for (i = pipeOneIndex+1; i < size; i++)
		{
			if (i+1 == size)
			{
				printf("%s\n", "finishing things off with a null");
				cmd2[i-cmd_1_size] = (char*) NULL;
			}
			else
			{
				printf("%s\n", "adding shit to cmd TWO BBY");
				cmd2[i-cmd_1_size] = (char*) calloc(strlen(instr_ptr->tokens[i]) + 1, sizeof(char));
				strcpy(cmd2[i-cmd_1_size], instr_ptr->tokens[i]);
				printf("cmd2 %d: %s\n", i-cmd_1_size, cmd2[i-cmd_1_size]);
			}
			cmd_2_size+=1;
		}

	}
	//handle command copies for two pipes
	else
	{
		// gotta populate cmd1 
		for (i = 0; i <= pipeOneIndex; i++)
		{
			if (i == pipeOneIndex)
			{
				printf("%s\n", "finishing things off with a null");
				cmd1[i] = (char*) NULL;
			}
			else
			{
				//printf("%s\n", "adding shit to cmd one");
				cmd1[i] = (char*) calloc(strlen(instr_ptr->tokens[i]) + 1, sizeof(char));
				strcpy(cmd1[i], instr_ptr->tokens[i]);
				printf("cmd1 %d: %s\n", i, cmd1[i]);
			}
			cmd_1_size+=1;
		}

		printf("\n");
		// gotta populate cmd2
		for (i = pipeOneIndex+1; i <= pipeTwoIndex; i++)
		{
			if (i == pipeTwoIndex)
			{
				printf("%s\n", "finishing things off with a null");
				cmd2[i-cmd_1_size] = (char*) NULL;
			}
			else
			{
				printf("%s\n", "adding shit to cmd TWO BBY");
				cmd2[i-cmd_1_size] = (char*) calloc(strlen(instr_ptr->tokens[i]) + 1, sizeof(char));
				strcpy(cmd2[i-cmd_1_size], instr_ptr->tokens[i]);
				printf("cmd2 %d: %s\n", i-cmd_1_size, cmd2[i-cmd_1_size]);
			}
			cmd_2_size+=1;
		}

		printf("\n");
		for (i = pipeTwoIndex+1; i < size; i++)
		{
			if (i+1 == size)
			{
				printf("%s\n", "finishing things off with a null");
				cmd3[i-(cmd_1_size+cmd_2_size+1)] = (char*) NULL;
			}
			else
			{
				printf("%s\n", "adding shit to cmd THREE YEE");
				cmd3[i-(cmd_1_size+cmd_2_size)] = (char*) calloc(strlen(instr_ptr->tokens[i]) + 1, sizeof(char));
				strcpy(cmd3[i-(cmd_1_size+cmd_2_size)], instr_ptr->tokens[i]);
				printf("cmd3 %d: %s\n", i-(cmd_1_size+cmd_2_size), cmd3[i-(cmd_1_size+cmd_2_size)]);
			}
			cmd_3_size+=1;
		}

	}


	printf("Num pipes: %d   Size: %d\n", numPipes, size);
	printf("Pipe One Index: %d   Pipe Two Index: %d\n", pipeOneIndex, pipeTwoIndex);
	printf("cmd_1_size: %d   cmd_2_size: %d   cmd_3_size: %d\n", cmd_1_size, cmd_2_size, cmd_3_size);


	// HANDLE ACTUAL PIPING HERE
	printf("\n\nPIPING STARTS HERE\n~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~\n\n");




	// handle a single pipe
	if(numPipes == 1)
	{
		int status;
		pid_t pid, pid2, pid3;
		int fd[2];
		printf("Attempting to fork\n");
		
		if (pid = fork() == 0)
		{
			// child (cmd1 | cmd2)
			pipe(fd);

			if (pid = fork() == 0)
			{
				//cmd1 (writer)
				printf("in cmd1 (writer)\n");
				close(STDOUT_FILENO);
				dup(fd[1]);
				close(fd[0]);
				close(fd[1]);
				// execute command
				execv(cmd1[0], cmd1);
			}
			else
			{				
				//cmd1 (writer)
				waitpid(pid, &status, 0);
				printf("in cmd2 (reader)\n");
				close(STDIN_FILENO);
				dup(fd[0]);
				close(fd[0]);
				close(fd[1]);
				// execute command
				execv(cmd2[0], cmd2);
			}
		}
		else
		{
			// parent (shell)
			printf("in big else\n");
			waitpid(pid, &status, 0);
			close(fd);
		}


	}

	// handle a double pipe
	else
	{
		int status;
		pid_t pid, pid2, pid3;
		int fd[2];
		int fd2[2];
		printf("Attempting to double pipe\n");

		//pipe(fd);
		//pipe(fd2);

		if (pid = fork() == 0)
		{
			// child (cmd1 | cmd2)
			pipe(fd);
			if (pid = fork() == 0)
			{
				printf("in cmd1 (writer)\n");
				close(STDOUT_FILENO);
				
				dup(fd[1]);

				close(fd[0]);
				close(fd[1]);
				// close(fd2[0]);
				// close(fd2[1]);
				// execute command
				printf("executing 1\n");
				execv(cmd1[0], cmd1);
			}
			else
			{			

				//waitpid(pid, &status, 0);	
				//pipe(fd2);
				if (fork() == 0)
				{
					//mapipe(fd2);
					waitpid(pid, &status, 0);	
					printf("in cmd2 (reader/writer)\n");
					close(STDIN_FILENO);
					close(STDOUT_FILENO);

					dup(fd[0]);
					pipe(fd2);
					dup(fd2[1]);

					close(fd[0]);
					close(fd[1]);
					close(fd2[0]);
					close(fd2[1]);

					// execute command
					printf("executing 2\n");
					execv(cmd2[0], cmd2);
				}
				else
				{		

					// for(i=0;i<3;i++)
					// {
					// 	waitpid(pid2, &status, 0);
					// 	waitpid(pid, &status, 0);
					// 	waitpid(pid3, &status, 0);
					// }

					waitpid(pid, &status, 0);
					//waitpid(pid, &status, 0);

					printf("in cmd3 (reader)\n");
					//close(STDIN_FILENO);

					dup(fd2[0]);
				
					close(fd[0]);
					close(fd[1]);
					close(fd2[0]);
					close(fd2[1]);	
					// execute command
					printf("executing 3\n");
					execv(cmd3[0], cmd3);
					printf("POST 3\n");
				}
			}

		}
		else
		{
			waitpid(pid, &status, 0);
			waitpid(pid, &status, 0);
			waitpid(pid, &status, 0);
			waitpid(pid, &status, 0);
			// parent (shell)
			printf("in big else\n");
			close(fd);
			close(fd2);
			//waitpid(pid, &status, 0);
			

		}
	}		




	printf("\n\n~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~\nPIPING ENDS HERE\n\n");

	//free all allocated memory
	for (i = 0; i < cmd_1_size; i++)
		free(cmd1[i]);
	free(cmd1);

	for (i = 0; i < cmd_2_size; i++)
		free(cmd2[i]);
	free(cmd2);

	for (i = 0; i < cmd_3_size; i++)
		free(cmd3[i]);
	free(cmd3);



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