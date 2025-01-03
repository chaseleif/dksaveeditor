CC:=gcc
CFLAGS:=-Iinclude -O2
CURSELIB:=-lncursesw
SRCDIR:=src
SRCS:=$(wildcard $(SRCDIR)/*.c)
OBJDIR:=obj
OBJS:=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
DATADIR:=data
DATALOADER:=dataloader
DATALOADERSRC:=$(DATADIR)/dataloader.c $(SRCDIR)/fileio.c $(SRCDIR)/util.c
DATAOUTPUT:=darklands.data
DATASRCIN:=$(DATADIR)/static_loader.c
DATASRCOUT:=$(SRCDIR)/darklands_data.c
BIN:=dksveditor

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(CURSELIB) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS): $(OBJDIR)

ifeq ($(origin DARKLANDS),undefined)
$(OBJDIR):
	mkdir $(OBJDIR)
else
$(OBJDIR):
	mkdir $(OBJDIR)
	$(CC) $(CFLAGS) $(DATALOADERSRC) -DNO_NCURSES -o $(DATALOADER)
	./$(DATALOADER) $(DARKLANDS)
	cp $(DATASRCIN) $(DATASRCOUT)
	xxd -i $(DATAOUTPUT) | head --lines=-1 >> $(DATASRCOUT)
	rm -f $(DATALOADER) $(DATAOUTPUT)
endif

.PHONY: clean
clean:
	rm -rf $(OBJDIR)
