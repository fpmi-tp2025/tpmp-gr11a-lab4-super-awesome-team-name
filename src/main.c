/* main.c */
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>

#include "../include/interface.h"

typedef enum { COMMANDER, CREW_MEMBER } Position;

typedef struct {
    int tab_number;
    Position position;
} User;

int main() {
    sqlite3 *db;
    int tab_number;
    printf("Добро пожаловать в управление отрядом групповых вертолетов!\n"
           "Если вы хотите зайти за командира, то ваш табельный номер = 1\n"
           "Если за обычного служащего, то ваш табельный номер = 2-12\n");

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
    User user = {tab_number, CREW_MEMBER}; // по умолчанию - член экипажа

    const char *sql = "SELECT position FROM Crew_member WHERE tab_number = ?";

    // Подготовка запроса
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
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
    } else {
        // Если пользователь не найден в базе данных, выводим сообщение
        printf("Пользователь с таким табельным номером не найден.\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;  // Завершаем программу
    }

    // Завершение работы с запросом
    sqlite3_finalize(stmt);

    if (user.position == COMMANDER) {

    } else if (user.position == CREW_MEMBER) {

    }

    // Определяем должность и показываем соответствующий интерфейс
    if (user.position == COMMANDER) {
        printf("\nВы находитесь в интерфейсе для командира.\n");
        commander_interface(db);
    } else if (user.position == CREW_MEMBER) {
        printf("Вы находитесь в интерфейсе для члена экипажа.\n");
        crew_member_interface(db, tab_number);
    }

    // Закрытие базы данных
    sqlite3_close(db);

    return 0;
}
