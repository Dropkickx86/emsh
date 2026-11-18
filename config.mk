CC = cc
CFLAGS = -D_POSIX_C_SOURCE=202405L -std=c23 -O2 -pedantic -Wall -Wextra
CFLAGS_DEBUG = -O0 -g -fno-omit-frame-pointer
#CFLAGS_DEBUG += -fanalyzer -fsanitize=address,undefined
LDFLAGS = -lm

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man