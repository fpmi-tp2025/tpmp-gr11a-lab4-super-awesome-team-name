/* crew.h */
#ifndef CREW_H
#define CREW_H

#include <sqlite3.h>
#include "models/crew_struct.h"

// Функция для получения информации о члене экипажа по его табельному номеру
CrewMember* get_crew_member_report(sqlite3 *db, int tab_number);

// Функция для получения информации о вертолете, закрепленном за членом экипажа
HelicopterInfo retrieve_helicopter_info_data(sqlite3 *db, int tab_number);

// Функция для получения налетанных часов и оставшегося ресурса вертолета члена экипажа
void get_flight_hours_for_crew_helicopter(sqlite3 *db, int tab_number);

// Функция для получения данных о рейсах вертолета члена экипажа за указанный период
void get_flights_by_period_for_crew(sqlite3 *db, int tab_number);

// Функция для расчета денег, заработанных членом экипажа за определенный период
void calculate_crew_member_earnings(sqlite3 *db, int tab_number, const char *start_date, const char *end_date);

// Функция для расчета денег, заработанных членом экипажа за конкретный рейс
void calculate_crew_member_earnings_for_flight(sqlite3 *db, int tab_number, int flight_code);

// Функция для получения информации о всех рейсах члена экипажа
void get_all_flights_for_crew(sqlite3 *db, int tab_number);

// Функция для обновления личной информации члена экипажа
int update_crew_member_info(sqlite3 *db, int tab_number);

#endif // CREW_H