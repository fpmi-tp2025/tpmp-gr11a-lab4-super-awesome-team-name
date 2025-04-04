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
FlightHoursInfo retrieve_flight_hours_data_crew(sqlite3 *db, int tab_number);

// Функция для получения данных о рейсах вертолета члена экипажа за указанный период
FlightReport retrieve_flights_report(sqlite3 *db, int tab_number, const char *start_date, const char *end_date);

// Функция для расчета денег, заработанных членом экипажа за определенный период
EarningsReport retrieve_earnings_data(sqlite3 *db, int tab_number, const char *start_date, const char *end_date);

// Функция для расчета денег, заработанных членом экипажа за конкретный рейс
EarningsRecordForFlight retrieve_earnings_data_for_flight(sqlite3 *db, int tab_number, int flight_code);

// Функция для получения информации о всех рейсах члена экипажа
FlightReport2 retrieve_all_flights_data(sqlite3* db, int tab_number);

// Функция для обновления личной информации члена экипажа
void update_crew_member_info(sqlite3 *db, int tab_number);

#endif // CREW_H