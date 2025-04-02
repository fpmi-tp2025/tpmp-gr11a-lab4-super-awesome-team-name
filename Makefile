# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -std=c99

# Имя исполнимого файла
TARGET = main

# Исходные файлы
SRC = src/main.c src/commander.c src/crew.c src/interface.c src/validation.c

# Тестовые файлы
TEST_SRC = tests/test_validation.c
TEST_OBJ = $(TEST_SRC:tests/%.c=$(BIN_DIR)/%.o)
TEST_EXEC = $(TEST_SRC:tests/%.c=$(BIN_DIR)/%)

# Библиотеки
LIBS = -lsqlite3
CHECK_LIB = $(shell pkg-config --cflags --libs check) -lm -lpthread  # Получаем флаги для Check через pkg-config

# Директории для бинарных файлов и исходных
BIN_DIR = bin
SRC_DIR = src
TEST_DIR = tests

# Объектные файлы
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

# Правила для сборки
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(BIN_DIR)/$(TARGET) $(LIBS)

# Правила для компиляции исходников
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Правила для компиляции тестов
$(BIN_DIR)/test_validation: tests/test_validation.c src/validation.c
	@mkdir -p $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/test_validation tests/test_validation.c src/validation.c -I./src $(CHECK_LIB) -lm -lpthread -lsqlite3

$(BIN_DIR)/test_commander: tests/test_commander.c src/commander.c
	@mkdir -p $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/test_commander tests/test_commander.c src/commander.c src/validation.c -I./src $(CHECK_LIB) -lm -lpthread -lsqlite3


# Сборка и запуск тестов
test: $(BIN_DIR)/test_validation $(BIN_DIR)/test_commander
	$(BIN_DIR)/test_validation
	$(BIN_DIR)/test_commander

# Очистка
clean:
	rm -rf $(BIN_DIR)/*.o $(BIN_DIR)/$(TARGET) $(BIN_DIR)/test_validation

# Правило по умолчанию
all: $(TARGET)
