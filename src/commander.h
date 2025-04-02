/* commander.h  */
#ifndef COMMANDER_H
#define COMMANDER_H

#include <sqlite3.h>

// структура для get_max_earning_crew
typedef struct {
    int helicopter_number;
    char helicopter_model[128];
    double total_earnings;
    struct {
        int tab_number;
        char last_name[128];
    } crew_members[10];
    int crew_count;
    struct {
        char date[128];
        int flight_code;
        double flight_cost;
    } flights[10];
    int flight_count;
} max_earning_crew_t;

// Функция для получения и вывода данных по рейсам в указанном периоде
void get_flights_data_by_period(sqlite3 *db);

// Функция для получения налетанных часов и ресурса летного времени после капитального ремонта
void get_flights_hours_after_repair(sqlite3 *db);

// Функция для получения общего количества рейсов, массы грузов и суммы заработанных денег по спецрейсам
void get_special_flights_summary(sqlite3 *db);

// Функция для вывода максимально заработавшего экипажа
max_earning_crew_t get_max_earning_crew(sqlite3 *db);

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
