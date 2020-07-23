# CC = gcc
CFLAGS = -Wall -g   

A3OBJS = diskinfo.o disklist.c helpers.c

all: diskinfo disklist

assign3: $(A3OBJS)
	$(CXX) $(CFLAGS) -o diskinfo disklist helpers $(A3OBJS) $(LDFLAGS) 

clean: 
	rm -rf $(A2OBJS) diskinfo disklist helpers
