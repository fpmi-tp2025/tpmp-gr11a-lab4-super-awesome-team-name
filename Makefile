SRC_DIR = src
BIN_DIR = bin
EXECUTABLE = $(BIN_DIR)/main_program
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lcmocka

TESTS_DIR = tests
TESTS_SRC = $(wildcard $(TESTS_DIR)/*.c)
TESTS_OBJ = $(TESTS_SRC:$(TESTS_DIR)/%.c=$(BIN_DIR)/%.o)

# Папка bin
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Правило для компиляции исполнимого файла (основной код)
$(EXECUTABLE): $(OBJ) $(BIN_DIR)
	$(CC) $(OBJ) -o $(EXECUTABLE) $(LDFLAGS)

# Правило для компиляции исходных файлов в объектные файлы (основной код)
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Правило для компиляции тестов в объектные файлы
$(BIN_DIR)/%.o: $(TESTS_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Запуск тестов
run_tests: $(TESTS_OBJ)
	$(CC) $(TESTS_OBJ) -o $(BIN_DIR)/test_program $(LDFLAGS)
	$(BIN_DIR)/test_program

# Очистка
clean:
	rm -rf $(BIN_DIR)

.PHONY: clean run_tests
