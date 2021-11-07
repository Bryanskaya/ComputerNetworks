CPPFLAGS=-g -pthread -I/sw/include/root
LDFLAGS=-g -pthread
LDLIBS=-L/sw/lib/root -lCore
SRCS=server.cpp
OBJS=$(subst .cpp,.o,$(SRCS))
EXES=$(subst .cpp, ,$(SRCS))

all: server

server: server.o
	g++ $(LDFLAGS) -o server server.o

server.o: server.cpp includes.hpp
	g++ $(CPPFLAGS) -c server.cpp

RM=rm -f
clean:
	$(RM) $(OBJS) $(EXES)