/* validation.c */

#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>

// Функция для проверки, является ли строка валидным именем
int validate_name(const char *name) {
    while (*name) {
        if ((*name < 'A' || *name > 'Z') && (*name < 'a' || *name > 'z')) {
            return 0;
        }
        name++;
    }
    return 1;
}

// Функция для проверки корректности формата даты (YYYY-MM-DD)
int validate_date(const char *date_str) {
    struct tm tm;
    return strptime(date_str, "%Y-%m-%d", &tm) != NULL;
}

// Функция для проверки корректности значения для позиции (commander или crew_member)
int validate_position(const char *position) {
    return (strcmp(position, "commander") == 0 || strcmp(position, "crew_member") == 0);
}

// Функция для проверки корректности года рождения (должен быть 4 цифры)
int validate_birth_year(const char *year_str) {
    if (strlen(year_str) != 4) return 0;
    for (int i = 0; i < 4; i++) {
        if (year_str[i] < '0' || year_str[i] > '9') {
            return 0;
        }
    }
    return 1;
}

// Функция для проверки наличия вертолета в базе данных
int validate_helicopter_number(sqlite3 *db, int helicopter_number) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM Helicopter WHERE helicopter_number = ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, helicopter_number);

    int count = 0;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return (count > 0);
}