# Makefile for usermux

dir := usermux
makemode := server

target = usermux

SRCS = usermux.c mux.c leaf.c node.c stubs.c

OBJS = $(SRCS:.c=.o)
HURDLIBS = netfs fshelp iohelp ports ihash shouldbeinlibc
LDLIBS = -lpthread

include ../Makeconf
