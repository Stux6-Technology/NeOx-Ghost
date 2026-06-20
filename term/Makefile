#   Copyright (C) 1995,96,97,99, 2000, 2002, 2012 Free Software Foundation,
#   Inc.
#
#   Copyright (C) 2026 Stux6 Technology Team 

dir := term
makemode := server

target = term
SRCS = devio.c munge.c users.c main.c ptyio.c hurdio.c xinl.c

HURDLIBS = trivfs fshelp iohelp ports ihash shouldbeinlibc
LDLIBS = -lpthread
OBJS = $(subst .c,.o,$(SRCS)) termServer.o device_replyServer.o tioctlServer.o ourmsgUser.o

include ../Makeconf

device_replyServer-CPPFLAGS = -DTypeCheck=0 -Wno-unused # XXX

device_reply-MIGSFLAGS=\
    "-DMACH_PAYLOAD_TO_PORT=ports_payload_get_name" \
    "-DDEVICE_IMPORTS=import \"$(srcdir)/../libports/ports.h\";"
tioctl-MIGSFLAGS = -imacros $(srcdir)/mig-mutate.h
term-MIGSFLAGS = -imacros $(srcdir)/mig-mutate.h
