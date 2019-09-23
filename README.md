# Project 1: Implementing a Shell


## Team Members

Keaun Moughari 

Marlan McInnes-Taylor 

Hayden Rogers

## List of Files and Descriptions
* shell.c - source code for Bash emulator

We have everything included in one file.
Within this file all function declarations can be found at the top.
There is a lot of documentation within the code to help aid in the grading process.

## Makefile description / How to Run

Once untaring the files, the user will just need to type 'make' and then type 'shell' and hit enter.

## Documentation of Group Member Contribution

### Meetings
* Group meeting 9/4 - discussed project plan - everyone present
* Group meeting 9/5 - discussed completion of tasks 1, 2, 3. Everyone working on part 4 for practice - everyone present
* Group meeting 9/8 - compared plans for tackling part 5 and 6 - everyone present
* Group meeting 9/11 - compared updates and made future plans for handling needed functions - everyone present
* Group meeting 9/15 - heavy coding session. mapped out a lot of things. resolved bugs. wrote funcs. - everyone present
* Group meeting 9/19 - We discussed what code we had written and implemented a plan for future funcs - everyone present
* Group meeting 9/20 - We met and coded in a group on each other machines. Impemented and made commits - everyone present
* Group meeting 9/21 - ALL DAY CODING MARATHON - all members present
* Group meeting 9/22 - ALL DAY CODING MARATHON - all members present

### Division of Labor
Due to the nature of this assignment much of the labor was completed in person with all members present for the first portions of the project. In order to learn and gain practice we met frequently and bouced ideas off of each other while trying our own approaches and meeting back to land on what we considered to be the best take. 

In each group meeting we all mapped out what we wanted to do and discussed how we wanted to divide everything up.
We all got to know each others machines and we worked together to get things implemented. Regarding parts 1-4; we
met and worked together to plan and decide how to implement ideas. For the rest of the project we began to branch
and focus on different areas with Keaun working on parsing functions, expanding paths and a lot of the serious core functionality, Marlan working on a lot of the i/o redirection, some of the built-ins and execution 
statements, and Hayden working on some expand functionality as well as piping and some execution.

All members rate all other members a 10/10 would group again.

### Git commit log

The git commit log is inluded as a .txt file within the tar.

### Assessment of each task/section & known bugs
1. Parsing:
  Used provided parser which was eventually implemented as a separate function. Updated parser to treat the '=' as operator character in order to ease parsing of alias commands. Updated parser to manipulate standard input streams to assist in alias expansion.
  Known Bugs: None

2. Environment Variables:
  Enviroment variables are correctly expanded.  

3. Prompt:
  There are know known bugs with the prompt.
  We do have an optional welcom screen we highly recommend
  using as it makes the whole experience more fun.
  It is commented out by default in case it fell under
  the category of "unecessary print statement"

4. Shortcut Resolution:
  We have no known bugs with shortcut resolution.

5. Path Resolution
  We have no known bugs with Path resolution.

6. Execution
  As for executing single commands within the context of this requirement,
  we have no known bugs.

7. I/O Redirection
  No known bugs with I/O redirection.

8. Piping:
  Piping works with some commands we wrote but not all commands.
  For example, in simple i/o files we wrote for testing purposes
  we were able to have piping work for single pipes, but then it 
  would fail when we tried to do some other commands.

  An example would be t1 | t2 which worked perfectly, but something
  like sort test.txt | uniq would not function properly. 

  For double piping, we were able to get some operations working in
  the correct order, but then sometimes it would fail -- and sometimes
  it would appear to access unexpected areas of the code. 

  We tried a variety of different approaches here but were unable to
  procure a solution that worked for everything.

9. Background processing:
  We did not have time to implement background processing as we had
  too many other fires to put out. We weren't able to dicuss as a
  team how we wanted to handle it so we built our functions under
  the assumption we would worry about it later.

10. Built-Ins:
* exit
  Outside of things tied to the background processing requirement
  we have no known bugs for exit.
* cd
  No known bugs for CD.
* echo
  No known bugs for echo.
* alias
  The program will SAVE the alius as whatever it is, but
  we do not SAVE the alius properly.
* unalias
  There are no known bugs with unalias.


KNOWN BUGS NOT MENTIONED ABOVE:

* if the input ends with a space character, the program will hang.
* piping is partially broken. more information above under piping.
* background processing is not implemented.

### EXTRA CREDIT

We completed the following extra credit:

* Expansion
* Environment Variables
* SHellception
