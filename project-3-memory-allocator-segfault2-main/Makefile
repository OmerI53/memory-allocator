# Add additional .c files here if you added any yourself.
ADDITIONAL_SOURCES =

# Add additional .h files here if you added any yourself.
ADDITIONAL_HEADERS =

# -- Do not modify below this point - will get replaced during testing --
CURDIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

LIB = libkualloc.so
SOURCES_LIB = alloc.c $(ADDITIONAL_SOURCES)
HEADERS_LIB = alloc.h $(ADDITIONAL_HEADERS)
TEST = test
SOURCES_TEST = test_framework/main.c test_framework/tests.c \
			   test_framework/memlist.c test_framework/checked_alloc.c \
			   test_framework/intercept.c
HEADERS_TEST = test_framework/common.h test_framework/tests.h \
			   test_framework/memlist.h test_framework/checked_alloc.h
META = Makefile README.rst check.py

CFLAGS = -Wall -Wextra -std=gnu99 -MD -g3 -O1
CFLAGS_LIB = -fPIC
CFLAGS_TEST = -DALLOC_TEST_FRAMEWORK
LDFLAGS =
LDFLAGS_LIB = -fPIC -shared
LDFLAGS_TEST = -ldl -L. -Wl,-rpath="$(CURDIR)" -lkualloc

CC = gcc

.SUFFIXES: # Disable built-in rules
.PHONY: all clean check

all: $(LIB) $(TEST)

check: all
	@./check.py

clean:
	rm -f $(LIB)
	rm -f $(TEST)
	rm -f *.o test_framework/*.o
	rm -f *.d test_framework/*.d

$(LIB): $(SOURCES_LIB:.c=.o)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDFLAGS_LIB)

$(TEST): $(SOURCES_TEST:.c=.o) | $(LIB)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDFLAGS_TEST)

%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_LIB) -c -o $@ $<
test_framework/%.o: test_framework/%.c
	$(CC) $(CFLAGS) $(CFLAGS_TEST) -c -o $@ $<

-include *.d
