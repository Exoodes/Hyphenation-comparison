# Simple Makefile for compare program, which is used for testing Judy 
# performance and for hyphenator program which is used to to hyphenate words
# using hyphenation patterns

# time command path
TIME_PATH := /usr/bin/time

# Folder names
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Compiler
CC := gcc

# Flags for Compiler and linker
CPPFLAGS := -Iinclude -MMD -MP 
CFLAGS   := -Wall -D_REENTRANT -D_XOPEN_SOURCE=500 -ggdb3
LDLIBS   := -lJudy -lcprops

# Variables for compare program
EXE_COMPARE := $(BIN_DIR)/compare
SRC_COMPARE := $(SRC_DIR)/compare.c $(SRC_DIR)/patterns.c $(SRC_DIR)/judy.c $(SRC_DIR)/trie.c $(SRC_DIR)/utils.c 
OBJ_COMPARE := $(SRC_COMPARE:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Inputs for compare program
INPUT_HR := croatian
INPUT_CZ := czech
INPUT_DA := danish
INPUT_NL := dutch
INPUT_EN := english
INPUT_FR := french
INPUT_KA := georgian
INPUT_DE := german
INPUT_IT := italian
INPUT_NB := norwegian
INPUT_PL := polish
INPUT_PT := portuguese
INPUT_RU := russian
INPUT_ES := spanish
INPUT_TH := thai
INPUT_TR := turkish
INPUT_UK := ukrainian
INPUT_COMBINED := combined

# Change INPUT_LANGUAGE variable for any other language input from above, for 
# example: INPUT_LANGUAGE := $(INPUT_UK) for ukrainian patterns and words
INPUT_LANGUAGE := $(INPUT_TH)
INPUT := assets/$(INPUT_LANGUAGE)_patterns.pat assets/$(INPUT_LANGUAGE)_words.dic

# Variables for hyphenator program
EXE_HYPHENATOR := $(BIN_DIR)/hyphenator
SRC_HYPHENATOR := $(SRC_DIR)/hyphenator.c $(SRC_DIR)/patterns.c $(SRC_DIR)/judy.c $(SRC_DIR)/utils.c 
OBJ_HYPHENATOR := $(SRC_HYPHENATOR:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
INPUT_HYPHENATOR := -l2 -r2 -f assets/$(INPUT_LANGUAGE)_words.dic assets/$(INPUT_LANGUAGE)_patterns.pat

all: $(EXE_COMPARE) $(EXE_HYPHENATOR)

.PHONY: all clean run-tests time-test memory-test hyphenator

$(EXE_COMPARE): $(OBJ_COMPARE) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXE_HYPHENATOR): $(OBJ_HYPHENATOR) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

run-tests: time-test memory-test $(EXE_COMPARE)

time-test: $(EXE_COMPARE)
	@echo "Time testing with $(INPUT_LANGUAGE) language"
	@$(EXE_COMPARE) -v -i $(INPUT)
	@$(EXE_COMPARE)  $(INPUT)

memory-test: $(EXE_COMPARE)
	@echo "Memory testing with $(INPUT_LANGUAGE) language"
	@echo "\nWith valgrind"
	@valgrind $(EXE_COMPARE) $(INPUT) -p > tmpfile.txt 2>&1 ; echo -n "Only patterns : " ; grep "total heap usage" tmpfile.txt
	@valgrind $(EXE_COMPARE) $(INPUT) -j > tmpfile.txt 2>&1 ; echo -n "Judy          : " ; grep "total heap usage" tmpfile.txt
	@valgrind $(EXE_COMPARE) $(INPUT) -t > tmpfile.txt 2>&1 ; echo -n "Trie          : " ; grep "total heap usage" tmpfile.txt
	@echo "\nWith time command"
	@$(TIME_PATH) --verbose $(EXE_COMPARE) $(INPUT) -p > tmpfile.txt 2>&1 ; echo -n "Only patterns : " ; grep "Maximum resident set size" tmpfile.txt
	@$(TIME_PATH) --verbose $(EXE_COMPARE) $(INPUT) -j > tmpfile.txt 2>&1 ; echo -n "Judy          : " ; grep "Maximum resident set size" tmpfile.txt
	@$(TIME_PATH) --verbose $(EXE_COMPARE) $(INPUT) -t > tmpfile.txt 2>&1 ; echo -n "Trie          : " ; grep "Maximum resident set size" tmpfile.txt
	@rm tmpfile.txt

hyphenator: $(EXE_HYPHENATOR)
	$(EXE_HYPHENATOR) $(INPUT_HYPHENATOR)

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ_COMPARE:.o=.d)
-include $(OBJ_HYPHENATOR:.o=.d)
