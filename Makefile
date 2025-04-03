# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -std=c99 -I./include

# Имя исполнимого файла
TARGET = main

# Директории
BIN_DIR = bin
SRC_DIR = src
TEST_DIR = tests
INCLUDE_DIR = include

# Исходные файлы
SRC_MAIN = $(SRC_DIR)/main.c
SRC_INTERFACE = $(SRC_DIR)/interface.c
SRC_COMMANDER = $(SRC_DIR)/commander/commander.c $(SRC_DIR)/commander/commander_interface.c
SRC_CREW = $(SRC_DIR)/crew/crew.c $(SRC_DIR)/crew/crew_interface.c
SRC_VALIDATION = $(SRC_DIR)/validation.c

# Объединяем все исходные файлы
SRC = $(SRC_MAIN) $(SRC_COMMANDER) $(SRC_CREW) $(SRC_VALIDATION) $(SRC_INTERFACE)

# Тестовые файлы
TEST_SRC = $(TEST_DIR)/test_validation.c
TEST_EXEC = $(BIN_DIR)/test_validation

# Библиотеки
LIBS = -lsqlite3
CHECK_LIB = $(shell pkg-config --cflags --libs check) -lm -lpthread

# Объектные файлы
OBJ_MAIN = $(BIN_DIR)/main.o
OBJ_INTERFACE = $(BIN_DIR)/interface.o
OBJ_COMMANDER = $(BIN_DIR)/commander/commander.o $(BIN_DIR)/commander/commander_interface.o
OBJ_CREW = $(BIN_DIR)/crew/crew.o $(BIN_DIR)/crew/crew_interface.o
OBJ_VALIDATION = $(BIN_DIR)/validation.o

# Объединяем все объектные файлы
OBJ = $(OBJ_MAIN) $(OBJ_COMMANDER) $(OBJ_CREW) $(OBJ_VALIDATION) $(OBJ_INTERFACE)

# Правило по умолчанию
all: directories $(TARGET)

# Создание директорий для объектных файлов
directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BIN_DIR)/commander
	@mkdir -p $(BIN_DIR)/crew

# Правила для сборки
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(BIN_DIR)/$(TARGET) $(LIBS)

# Правила для компиляции main.c
$(OBJ_MAIN): $(SRC_MAIN)
	$(CC) $(CFLAGS) -c $< -o $@

# Правила для компиляции interface.c
$(OBJ_INTERFACE): $(SRC_INTERFACE)
	$(CC) $(CFLAGS) -c $< -o $@

# Правила для компиляции commander
$(BIN_DIR)/commander/%.o: $(SRC_DIR)/commander/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Правила для компиляции crew
$(BIN_DIR)/crew/%.o: $(SRC_DIR)/crew/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Правила для компиляции validation.c
$(OBJ_VALIDATION): $(SRC_VALIDATION)
	$(CC) $(CFLAGS) -c $< -o $@

# Правила для компиляции и запуска тестов
$(TEST_EXEC): $(TEST_SRC) $(SRC_VALIDATION)
	$(CC) -o $(TEST_EXEC) $(TEST_SRC) $(SRC_VALIDATION) -I$(INCLUDE_DIR) $(CHECK_LIB) $(LIBS)

# Сборка и запуск тестов
test: directories $(TEST_EXEC)
	$(TEST_EXEC)

# Очистка
clean:
	rm -rf $(BIN_DIR)

# Сборка и запуск
run: all
	./$(BIN_DIR)/$(TARGET)

.PHONY: all directories clean test run