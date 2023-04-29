# Dictionaries
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Compiler
CC := gcc

# Flags for Compile and linker
CPPFLAGS := -Iinclude -MMD -MP 
CFLAGS   := -Wall -D_REENTRANT -D_XOPEN_SOURCE=500 -ggdb3           
LDLIBS   := -lJudy -lcprops -lpthread -lssl

# Variables for compare program
EXE_COMPARE := $(BIN_DIR)/compare
SRC_COMPARE := $(SRC_DIR)/compare.c $(SRC_DIR)/patterns.c $(SRC_DIR)/judy.c $(SRC_DIR)/trie.c $(SRC_DIR)/utils.c 
OBJ_COMPARE := $(SRC_COMPARE:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Inputs for testing compare program
INPUT_HR := croatian
INPUT_CZ := czech #
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

# Change INPUT_NAME variable for any other language input, for exmaple 
# INPUT_NAME := $(INPUT_UK) for ukrainian patterns and words
INPUT_NAME := $(INPUT_UK)
INPUT := assets/$(INPUT_NAME)_patterns.tex assets/$(INPUT_NAME)_words.dic

# Variables for hyphenator program
EXE_HYPHENATOR := $(BIN_DIR)/hyphenator
SRC_HYPHENATOR := $(SRC_DIR)/hyphenator.c $(SRC_DIR)/patterns.c $(SRC_DIR)/judy.c $(SRC_DIR)/utils.c 
OBJ_HYPHENATOR := $(SRC_HYPHENATOR:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
INPUT_HYPHENATOR := -l2 -r2 -f assets/czech_words.dic assets/czech_patterns.tex

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

run-tests: time-test space-test $(EXE_COMPARE)

time-test: $(EXE_COMPARE)
	@echo "Time testing with $(INPUT_NAME) language"
	@$(EXE_COMPARE) $(INPUT)

memory-test: $(EXE_COMPARE)
	@echo "Memory testing with $(INPUT_NAME) language"
	@echo "\nWith valgrind"
	@valgrind $(EXE_COMPARE) $(INPUT) -p > tmpfile.txt 2>&1 ; echo -n "Only patterns : " ; grep "total heap usage" tmpfile.txt
	@valgrind $(EXE_COMPARE) $(INPUT) -j > tmpfile.txt 2>&1 ; echo -n "Judy          : " ; grep "total heap usage" tmpfile.txt
	@valgrind $(EXE_COMPARE) $(INPUT) -t > tmpfile.txt 2>&1 ; echo -n "Trie          : " ; grep "total heap usage" tmpfile.txt
	@echo "\nWith time command"
	@time --verbose $(EXE_COMPARE) $(INPUT) -p > tmpfile.txt 2>&1 ; echo -n "Only patterns : " ; grep "Maximum resident set size" tmpfile.txt
	@time --verbose $(EXE_COMPARE) $(INPUT) -j > tmpfile.txt 2>&1 ; echo -n "Judy          : " ; grep "Maximum resident set size" tmpfile.txt
	@time --verbose $(EXE_COMPARE) $(INPUT) -t > tmpfile.txt 2>&1 ; echo -n "Trie          : " ; grep "Maximum resident set size" tmpfile.txt
	@rm tmpfile.txt

hyphenator: $(EXE_HYPHENATOR)
	$(EXE_HYPHENATOR) $(INPUT_HYPHENATOR)

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ_COMPARE:.o=.d)
-include $(OBJ_HYPHENATOR:.o=.d)