SOURCE	= diskinfo.c disklist.c diskget.c diskput.c helpers.c
OUT     = diskinfo disklist diskget diskput
DEPS = Constants.h helpers.h
HELPER = helpers.c
CC	 = gcc
FLAGS	 = -Wall -lm

all: diskinfo disklist diskget diskput

diskinfo: diskinfo.c $(HELPER)
	$(CC) $(FLAGS) -o $@ $(HELPER) diskinfo.c

disklist: disklist.c $(HELPER)
	$(CC) $(FLAGS) -o $@ $(HELPER) disklist.c 

diskget: diskget.c $(HELPER)
	$(CC) $(FLAGS) -o $@ $(HELPER) diskget.c 

diskput: diskput.c $(HELPER)
	$(CC) $(FLAGS) -o $@ $(HELPER) diskput.c 



clean:
	rm -f $(OUT)
