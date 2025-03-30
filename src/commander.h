/* commander.h  */
#ifndef COMMANDER_H
#define COMMANDER_H

#include <sqlite3.h>

// Функция для проверки корректности формата даты (YYYY-MM-DD)
int validate_date(const char *date_str);

// Функция для получения и вывода данных по рейсам в указанном периоде
void get_flights_data_by_period(sqlite3 *db);

// Функция для получения налетанных часов и ресурса летного времени после капитального ремонта
void get_flights_hours_after_repair(sqlite3 *db);

// Функция для получения общего количества рейсов, массы грузов и суммы заработанных денег по спецрейсам
void get_special_flights_summary(sqlite3 *db);

// Функция для вывода максимально заработавшего экипажа
void get_max_earning_crew(sqlite3 *db);

// Информацию по вертолету и экипажу с макс кол-во рейсов
void get_helicopter_with_most_flights(sqlite3 *db);

// Данные по вертолетам проводившие обычный рейс
void get_normal_flights_summary(sqlite3 *db);

#endif // COMMANDER_H

