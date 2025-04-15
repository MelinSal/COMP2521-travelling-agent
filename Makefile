# COMP2521 24T3 - Assignment 2

# List all your supporting .c files here. Do NOT include .h files in this list.
# Note that when you submit, you must submit your supporting .c AND .h files.
# Example: SUPPORTING_FILES = hello.c world.c

SUPPORTING_FILES = 

########################################################################
# !!! DO NOT MODIFY ANYTHING BELOW THIS LINE !!!

CC = clang
CFLAGS = -Wall -Wvla -Werror -gdwarf-4

########################################################################

.PHONY: asan msan nosan

asan: CFLAGS += -fsanitize=address,leak,undefined
asan: all

msan: CFLAGS += -fsanitize=memory,undefined -fsanitize-memory-track-origins
msan: all

nosan: all

########################################################################

.PHONY: all
all: testMap game

testMap: testMap.c Map.c $(SUPPORTING_FILES)
	$(CC) $(CFLAGS) -o testMap testMap.c Map.c $(SUPPORTING_FILES)

game: game.c Agent.c Map.c $(SUPPORTING_FILES)
	$(CC) $(CFLAGS) -o game game.c Agent.c Map.c $(SUPPORTING_FILES)

.PHONY: clean
clean:
	rm -f testMap game

