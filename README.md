# Project 1: Implementing a Shell


## Team Members

Keaun Moughari 

Marlan McInnes-Taylor 

Hayden Rogers

## List of Files and Descriptions
* shell.c - source code for Bash emulator

## Makefile description / How to Run

## Documentation of Group Member Contribution

### Meetings
* Group meeting 9/4 - discussed project plan - everyone present
* Group meeting 9/5 - discussed completion of tasks 1, 2, 3. Everyone working on part 4 for practice - everyone present
* Group meeting 9/8 - compared plans for tackling part 5 and 6 - everyone present
* Group meeting 9/11 - compared updates and made future plans for handling needed functions - everyone present
* Group meeting 9/15 - hevy coding session. mapped out a lot of things. resolved bugs. wrote funcs. - everyone present

### Division of Labor
Due to the nature of this assignment much of the labor was completed in person with all members present for the first portions of the project. In order to learn and gain practice we met frequently and bouced ideas off of each other while trying our own approaches and meeting back to land on what we considered to be the best take. 

### Git commit log

### Needed Functions / Tests
* ```void expandVar(char* tok)``` - Hayden
    * Takes in C string of purposed variable (e.g., $HOME, $PWD, etc.)
    * Removes special character '$'
    * Use getenv() to expand variable into a temp char ptr
    * Calculate size of 'temp' contents and use realloc() on passed in var 'tok' to give any needed space (include room for '\0')
    * Use strcpy() or memcpy() to copy contents of 'temp' to 'tok'
    * Result should be modifying 'tok' as such: '$HOME' --> '/home/kroot'
    > Hayden >> I got this :^)

* ```void expandPath(char* tok)``` - Keaun 09/13
    * Takes in path as a C string
    * Use strtok() (there may be a better function) to break up each part of the path by '/' and store each in an index of temp (will probably need to be a double ptr and allocate using malloc()/calloc() for the appropriate size of each substring) 
    * Loop through temp and each occurence of an expandable substring (i.e., ~, .., .) should be expanded accordingly to the correct absolute path
    * Once all portions of the path have been expanded, use realloc() on 'tok' to allow enough room for all of 'temp' to be copied to it (strcpy() then strcat() will probably be used)
    * Free allocated memory used by 'temp'
    * Result should be modifying 'tok' as such: '../../../bin' --> '/bin' (Assume we were in /home/kroot/Code)
* ```int inPath(const char* tok)``` - Marlan 9/15
    * Initialize char* temp = "$PATH\0"
    * Use temp as argument for expandVar()
    * 'temp' will look something like this when returned: '/bin:/usr/bin:/usr/sbin'
    * Use strtok() with ':' as delimeter to break up each path and check each dir in path for the name of an executable that matches 'tok'
    * If no matches, print an error: " 'tok': command not found " 
* ```int isPath(const char* tok)``` - needs testing
* ```int isDir(const char* tok)```  - needs testing
* ```int isFile(const char* tok)```
    * Should be very similar to isDir(), except it uses S_ISREG() instead of S_ISDIR()
* ```int fileExists(const char* tok)``` - Hayden 9/15
    * Use open() with the flag O_EXCL and O_CREAT as such: open(tok, O_CREAT | O_EXCL)
    * If it fails, return 0. Otherwise, return 1


