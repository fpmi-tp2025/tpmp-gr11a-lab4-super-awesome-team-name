# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -std=c99

# Имя исполнимого файла
TARGET = helicopter_flights

# Исходные файлы
SRC = src/main.c src/commander.c src/crew.c

# Библиотеки
LIBS = -lsqlite3

# Директории для бинарных файлов и исходных
BIN_DIR = bin
SRC_DIR = src

# Объектные файлы
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

# Правила для сборки
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(BIN_DIR)/$(TARGET) $(LIBS)

# Правила для компиляции исходников
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	rm -rf $(BIN_DIR)/*.o $(BIN_DIR)/$(TARGET)

# Правило по умолчанию
all: $(TARGET)

