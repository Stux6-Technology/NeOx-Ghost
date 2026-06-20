# Makefile for tmpfs
#
#   Copyright (C) 2000,01,02,12 Free Software Foundation, Inc.
#
#   Copyright (C) 2026 Stux6 Technology Team

dir := tmpfs
makemode := server

target = tmpfs
SRCS = tmpfs.c node.c dir.c pager-stubs.c
OBJS = $(SRCS:.c=.o) default_pagerUser.o
# XXX The shared libdiskfs requires libstore even though we don't use it here.
HURDLIBS = diskfs pager iohelp fshelp store ports ihash shouldbeinlibc
LDLIBS = -lpthread

include ../Makeconf
