SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

CC := gcc

EXE := $(BIN_DIR)/compare

SRC := $(wildcard $(SRC_DIR)/*.c)

OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS := -Iinclude -MMD -MP 
CFLAGS   := -Wall #-ggdb3           
LDLIBS   := -lJudy

VALFLAGS := --show-leak-kinds=all --track-origins=yes --leak-check=full --track-fds=yes

INPUT_CZ  := assets/czech_patterns.tex assets/czech_testing_words.dic
INPUT_ENG := assets/english_patterns_max.tex assets/english_words.dic

all: $(EXE)

.PHONY: all clean run

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

run: $(EXE)
	$(EXE) $(INPUT_ENG)

valgrind: $(EXE)
	valgrind $(VALFLAGS) $(EXE) $(INPUT_ENG)

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)