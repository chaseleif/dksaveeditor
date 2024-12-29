CC:=gcc
CFLAGS:=-Iinclude -O2
CURSELIB:=-lncursesw
SRCDIR:=src
SRCS:=$(wildcard $(SRCDIR)/*.c)
OBJDIR:=obj
OBJS:=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
BIN:=dksveditor

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(CURSELIB) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS): $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

.PHONY: clean
clean:
	rm -rf $(OBJDIR)
