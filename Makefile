# DirWalker — Makefile
# C23 · GCC · sin dependencias externas

CC       := gcc
CFLAGS   := -std=c23 -Wall -Wextra -Wpedantic -O2 -D_GNU_SOURCE -Iinclude -MMD -MP
LDFLAGS  :=

SRCDIR   := src
OBJDIR   := build
BIN      := $(OBJDIR)/dirwalker

SRCS     := $(wildcard $(SRCDIR)/*.c)
OBJS     := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
DEPS     := $(OBJS:.o=.d)

.PHONY: all run debug clean

all: $(BIN)

$(BIN): $(OBJS) | $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

run: all
	./$(BIN)

debug: CFLAGS := -std=c23 -Wall -Wextra -Wpedantic -g -O0 -D_GNU_SOURCE -Iinclude -MMD -MP
debug: clean all

clean:
	rm -rf $(OBJDIR)

-include $(DEPS)
