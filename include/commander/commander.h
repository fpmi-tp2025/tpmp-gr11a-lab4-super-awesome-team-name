/* commander.h  */
#ifndef COMMANDER_H
#define COMMANDER_H

#include <sqlite3.h>
#include "models/commander_struct.h"
#include "../../include/validation.h"

// Функция для получения и вывода данных по рейсам в указанном периоде
FlightPeriodReport* get_flights_report(sqlite3 *db, const char *start, const char *end);

// Функция для получения налетанных часов и ресурса летного времени после капитального ремонта
HelicopterHours* retrieve_flight_hours_data(sqlite3 *db, int *result_count);

// Функция для получения общего количества рейсов, массы грузов и суммы заработанных денег по спецрейсам
SpecialFlightsSummary* retrieve_special_flights_data(sqlite3 *db, int *result_count);

// Функция для вывода максимально заработавшего экипажа
max_earning_crew_t retrieve_max_earning_crew_data(sqlite3 *db);

// Информацию по вертолету и экипажу с макс кол-во рейсов
HelicopterWithCrewData* get_helicopter_with_crew_most_flights(sqlite3 *db);

// Данные по вертолетам проводившие обычный рейс
HelicopterSummary* retrieve_normal_flights_data(sqlite3 *db, int* result_count);

PilotEarnings retrieve_pilot_earnings(sqlite3 *db, int pilot_id, const char* start_date, const char* end_date);

// Обновить данные экипажа
int update_crew_member_db(sqlite3 *db, int tab_number, const char *field, const char *new_value);

// Обновить данные о рейсе
int update_flight(sqlite3 *db);

// Обновить данные по вертолетам
int update_helicopter(sqlite3 *db);

// Вставка работника в БД
int insert_crew_member(sqlite3 *db);

// Удаление члена экипажа
int delete_crew_member(sqlite3 *db);

#endif // COMMANDER_H
