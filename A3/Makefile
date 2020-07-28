OBJS	= diskinfo.o disklist.o diskget.o diskput.o helpers.o
SOURCE	= diskinfo.c disklist.c diskget.c diskput.c helpers.c
HEADER	= Constants.h helpers.h
OUT	= a.out
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -lm

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

diskinfo.o: diskinfo.c
	$(CC) $(FLAGS) diskinfo.c -std=c99

disklist.o: disklist.c
	$(CC) $(FLAGS) disklist.c -std=c99

diskget.o: diskget.c
	$(CC) $(FLAGS) diskget.c -std=c99

diskput.o: diskput.c
	$(CC) $(FLAGS) diskput.c -std=c99

helpers.o: helpers.c
	$(CC) $(FLAGS) helpers.c -std=c99


clean:
	rm -f $(OBJS) $(OUT)
