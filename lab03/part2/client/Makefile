CPPFLAGS=-g -pthread -I/sw/include/root
LDFLAGS=-g
LDLIBS=-L/sw/lib/root -lCore
SRCS=server.cpp client.cpp
OBJS=$(subst .cpp,.o,$(SRCS))
EXES=$(subst .cpp, ,$(SRCS))

all: client

client: client.o
	g++ $(LDFLAGS) -o client client.o

client.o: client.cpp includes.hpp
	g++ $(CPPFLAGS) -c client.cpp

RM=rm -f
clean:
	$(RM) $(OBJS) $(EXES)