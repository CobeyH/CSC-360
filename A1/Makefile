CFLAGS = -Wall -g  
LDFLAGS = -L /usr/local/Cellar/readline/8.0.4/lib

OBJS = main.o

all: a1 

a1: $(OBJS)
	$(CC) $(CFLAGS) -o a1 $(OBJS) $(LDFLAGS) -lreadline -lhistory -ltermcap

clean: 
	rm -rf $(OBJS) a1 
