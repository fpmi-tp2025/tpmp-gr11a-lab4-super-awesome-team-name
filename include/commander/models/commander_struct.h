/*commander_struct.h*/
#ifndef TEAM_LAB4_COMMANDER_STRUCT_H
#define TEAM_LAB4_COMMANDER_STRUCT_H

typedef struct FlightPeriodReport {
    int helicopter_number;
    int *flight_codes;
    double *cargo_weights;
    int *passengers;
    int total_flights;
    double total_cargo;
    int total_passengers;
    struct FlightPeriodReport *next;
} FlightPeriodReport;


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

// структура для get_flights_hours_after_repair
typedef struct {
    int helicopter_number;
    double total_flight_hours;
    int flight_resource;
} HelicopterHours;

// структура для get_special_flights_summary
typedef struct {
    int helicopter_number;
    int flights_count;
    double total_cargo_weight;
    double total_income;
} SpecialFlightsSummary;

// Структура для хранения данных о вертолете
typedef struct {
    int helicopter_number;
    const char *helicopter_model;
    int num_flights;
    double total_earnings;
} HelicopterData;

// Структура для хранения данных о члене экипажа
typedef struct {
    int tab_number;
    const char *last_name;
} CrewMemberData;

// Структура для связи данных о вертолете и его экипаже
typedef struct {
    HelicopterData helicopter;
    CrewMemberData *crew_members;
    int crew_count;
} HelicopterWithCrewData;

// Структура для хранения данных о вертолете
typedef struct {
    int helicopter_number;
    char* model;
    int num_flights;
    double total_cargo_weight;
    double total_earnings;
} HelicopterSummary;

typedef struct {
    int pilot_id;         // Табельный номер летчика
    char* pilot_name;     // ФИО летчика
    double total_earnings; // Сумма заработанных денег
    int flight_count;     // Количество рейсов
} PilotEarnings;

#endif