/* commander.h  */
#ifndef COMMANDER_H
#define COMMANDER_H

#include <sqlite3.h>
#include "models/commander_struct.h"
#include "../../include/validation.h"

// Функция для получения и вывода данных по рейсам в указанном периоде
FlightPeriodReport* get_flights_report(sqlite3 *db, const char *start, const char *end);

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

// Обновить данные экипажа
int update_crew_member(sqlite3 *db);

// Обновить данные о рейсе
int update_flight(sqlite3 *db);

// Обновить данные по вертолетам
int update_helicopter(sqlite3 *db);

// Вставка работника в БД
int insert_crew_member(sqlite3 *db);

// Удаление члена экипажа
int delete_crew_member(sqlite3 *db);

#endif // COMMANDER_H
