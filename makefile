CC = gcc -g
RM = rm -f shell *.o output.txt

shell: shell.o
	$(CC) -o shell shell.o

shell.o: shell.c
	$(CC) -w -c shell.c
	
clean: 
	$(RM)
