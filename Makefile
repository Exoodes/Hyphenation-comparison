SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

CC := gcc

EXE := $(BIN_DIR)/compare

SRC := $(wildcard $(SRC_DIR)/*.c)

OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS := -Iinclude -MMD -MP # -I is a preprocessor flag, not a compiler flag
CFLAGS   := -Wall   -ggdb3           # some warnings about bad code
LDLIBS   := -lJudy

VALFLAGS := --show-leak-kinds=all --track-origins=yes --leak-check=full --track-fds=yes

INPUT := assets/english_patterns.tex assets/testing_words.dic

all: $(EXE) run

.PHONY: all clean run

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

run: $(EXE)
	$(EXE) $(INPUT)

valgrind: $(EXE)
	valgrind $(VALFLAGS) $(EXE) assets/english_patterns.tex assets/testing_words.dic

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)