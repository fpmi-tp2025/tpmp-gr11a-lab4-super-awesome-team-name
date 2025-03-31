/* validation.c */
#ifndef TEAM_LAB4_VALIDATION_H
#define TEAM_LAB4_VALIDATION_H

#include <sqlite3.h>

// Функция для проверки, является ли строка валидным именем
int validate_name(const char *name);

// Функция для проверки корректности формата даты (YYYY-MM-DD)
int validate_date(const char *date_str);

// Функция для проверки корректности значения для позиции (commander или crew_member)
int validate_position(const char *position);

// Функция для проверки корректности года рождения (должен быть 4 цифры)
int validate_birth_year(const char *year_str);

// Функция для проверки наличия вертолета в базе данных
int validate_helicopter_number(sqlite3 *db, int helicopter_number);

#endif //TEAM_LAB4_VALIDATION_H
