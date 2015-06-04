#bash/bin

TOPDIR=.

TARGE=Test

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp = .o)
CC = g++

#dirs :=        Kmp buf
#all:
#         $(foreach N,$(dirs),$(make -C $(N)))

#include header
INCLUDES = -I./

#include lib path
LIBS = 
LDLIBS = -lpthread

CCFLAGS = -g -Wall 

$(TARGE):$(OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(LDLIBS) $(CCFLAGS)

%.o:%.cpp
	$(CC) -c $< $(CCFLAGS)

clean:
	rm *.o $(TARGE)
