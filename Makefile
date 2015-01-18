CFLAGS=-O2 -Wall -Wextra -DNDEBUG $(OPTFLAGS)
PREFIX?=/usr/local

SOURCES=$(wildcard *.c)

SRC=stats.c

OBJS=$(SRC:.c=.o)
DEPS=$(SRC:.c=.d)

TARGET=desc
TIMINGSEXEC=timings
TESTSEXEC=test

all: $(TARGET) $(TESTEXEC) $(TIMINGSEXEC)

dev: CFLAGS=-g -Wall -Wextra $(OPTFLAGS)
dev: all

$(TARGET): $(OBJS) desc.o

$(TIMINGSEXEC): $(OBJS) timings.o

$(TESTSEXEC): $(OBJS) test.o

-include $(DEPS)

.PHONY: tests
tests: $(TESTSEXEC)
	./$(TESTSEXEC)

clean:
	rm -rf *.o $(TESTSEXEC) $(TARGET) $(TIMINGSEXEC)
	rm -rf *.dSYM

install: all
	install -d $(DESTDIR)/$(PREFIX)/bin/
	install $(TARGET) $(DESTDIR)/$(PREFIX)/bin/

BADFUNCS='[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)'
check:
	@echo Files with potentially dangerous functions.
	@egrep $(BADFUNCS) $(SOURCES) ||  true
