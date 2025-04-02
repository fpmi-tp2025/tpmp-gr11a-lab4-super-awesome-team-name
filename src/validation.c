/* validation.c */

#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>
#include "../include/validation.h"

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

// Проверка числового значения
int validate_number(const char *str) {
    while (*str) {
        if (*str < '0' || *str > '9') {
            return 0;
        }
        str++;
    }
    return 1;
}

// Проверка для чисел с плавающей точкой
int validate_float(const char *str) {
    int dot_count = 0;
    while (*str) {
        if (*str == '.') {
            dot_count++;
            if (dot_count > 1) return 0;
        } else if (*str < '0' || *str > '9') {
            return 0;
        }
        str++;
    }
    return 1;
}

// Проверка корректности yes/no значения для is_special
int validate_is_special(const char *input) {
    if (strcmp(input, "0") == 0 || strcmp(input, "нет") == 0) {
        return 0;
    }
    if (strcmp(input, "1") == 0 || strcmp(input, "да") == 0) {
        return 1;
    }
    return -1;
}

// Функция для проверки существования flight_code в базе данных
int validate_flight_code(sqlite3 *db, int flight_code) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM Flight WHERE flight_code = ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, flight_code);

    int count = 0;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return (count > 0);
}

// Функция для проверки существования члена экипажа по табельному номеру
int validate_crew_member(sqlite3 *db, int tab_number) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM Crew_member WHERE tab_number = ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, tab_number);

    int count = 0;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return (count > 0);  // Если член экипажа найден в базе
}
