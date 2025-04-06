/* validation.c */
#ifndef TEAM_LAB4_VALIDATION_H
#define TEAM_LAB4_VALIDATION_H

#include <sqlite3.h>

// Функция для проверки наличия вертолета в базе данных
int validate_helicopter_number(sqlite3 *db, int helicopter_number);

// Функция для проверки корректности формата даты (YYYY-MM-DD)
int validate_date(const char *date_str);

// Функция для проверки, является ли строка валидным именем
int validate_name(const char *name);

// Функция для проверки позиции (commander или crew_member)
int validate_position(const char *position);

// Функция для проверки корректности года рождения
int validate_birth_year(const char *year_str);

// Функция для проверки, является ли строка числом
int validate_number(const char *str);

// Функция для проверки, является ли строка числом с плавающей точкой
int validate_float(const char *str);

// Функция для проверки yes/no значения для is_special
int validate_is_special(const char *input);

// Функция для проверки существования flight_code в базе данных
int validate_flight_code(sqlite3 *db, int flight_code);

// Функция для проверки существования члена экипажа по табельному номеру
int validate_crew_member(sqlite3 *db, int tab_number);

// Проверка существования вертолёта
int validate_helicopter(sqlite3 *db, int helicopter_number);

// Проверка наличия членов экипажа, привязанных к вертолёту
int check_crew_members_for_helicopter(sqlite3 *db, int helicopter_number);

// Проверка наличия полётов, связанных с вертолётом
int check_flights_for_helicopter(sqlite3 *db, int helicopter_number);

#endif //TEAM_LAB4_VALIDATION_H
