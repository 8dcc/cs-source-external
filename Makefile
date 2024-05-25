
CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=-lX11 -lXext -lXfixes -lm

OBJ_FILES=main.c.o window.c.o util.c.o globals.c.o esp.c.o
OBJS=$(addprefix obj/, $(OBJ_FILES))

BIN=cs-source-external

#-------------------------------------------------------------------------------

.PHONY: clean all

all: $(BIN)

clean:
	rm -f $(OBJS)
	rm -f $(BIN)

#-------------------------------------------------------------------------------

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

obj/%.c.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
