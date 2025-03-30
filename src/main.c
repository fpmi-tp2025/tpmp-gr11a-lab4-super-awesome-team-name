/* main.c */
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include "db.h"
#include "interface.h"

typedef enum { COMMANDER, CREW_MEMBER } Position;

typedef struct {
    int tab_number;
    Position position;
} User;

int main() {
    sqlite3 *db;
    int tab_number;

    // Ввод табельного номера
    printf("Введите ваш табельный номер: ");
    scanf("%d", &tab_number);

    // Открытие базы данных
    int rc = sqlite3_open("air_carrier.db", &db);
    if (rc) {
        fprintf(stderr, "Не удалось открыть базу данных: %s\n", sqlite3_errmsg(db));
        return 1;  // Возвращаем ошибку, если база не открылась
    }

    sqlite3_stmt *stmt;
    User user = {0, CREW_MEMBER}; // по умолчанию - член экипажа

    const char *sql = "SELECT position FROM Crew_member WHERE tab_number = ?";

    // Подготовка запроса
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Привязка параметра
    sqlite3_bind_int(stmt, 1, tab_number);

    // Выполнение запроса
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *position = (const char*)sqlite3_column_text(stmt, 0);
        if (position != NULL && strcmp(position, "commander") == 0) {
            user.position = COMMANDER;
        }
    }

    // Завершение работы с запросом
    sqlite3_finalize(stmt);

    // Определяем должность и показываем соответствующий интерфейс
    if (user.position == COMMANDER) {
        commander_interface(db);
    } else if (user.position == CREW_MEMBER) {
        crew_member_interface(db);
    } else {
        printf("Пользователь с таким табельным номером не найден.\n");
    }

    return 0;
}
