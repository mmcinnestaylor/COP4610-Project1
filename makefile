CC = gcc
RM = rm -f shell *.o

shell: shell.o
	$(CC) -o shell shell.o

shell.o: shell.c
	$(CC) -c shell.c
	
clean: 
	$(RM)
