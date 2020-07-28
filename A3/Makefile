# CC = gcc
CFLAGS = -Wall -g   

A3OBJS = diskinfo.o disklist.c helpers.c diskget.c

all: diskinfo disklist helpers

assign3: $(A3OBJS)
	$(CXX) $(CFLAGS) -o diskinfo disklist helpers diskget $(A3OBJS) $(LDFLAGS) 

clean: 
	rm -rf $(A2OBJS) diskinfo disklist helpers diskget
