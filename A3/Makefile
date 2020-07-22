# CC = gcc
CFLAGS = -Wall -g   

A3OBJS = diskinfo.o disklist.c

all: diskinfo disklist

assign3: $(A3OBJS)
	$(CXX) $(CFLAGS) -o diskinfo disklist $(A3OBJS) $(LDFLAGS) 

clean: 
	rm -rf $(A2OBJS) diskinfo disklist
