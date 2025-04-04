/*crew_struct.h*/
#ifndef TEAM_LAB4_CREW_STRUCT_H
#define TEAM_LAB4_CREW_STRUCT_H

// get_crew_member_report
typedef struct {
    int tab_number;
    char* last_name;
    char* position;
    int experience_years;
    char* address;
    int birth_year;
    int helicopter_number;
} CrewMember;

// get_helicopter_info
typedef struct {
    int helicopter_number;
    char *model;
    char *manufacture_date;
    double max_payload;
    char *last_repair_date;
    int flight_resource;
    int found;
} HelicopterInfo;

// retrieve_flight_hours_data
typedef struct {
    int helicopter_number;
    double total_flight_hours;
    int flight_resource;
    int data_exists;
} FlightHoursInfo;

// retrieve_flights_report
typedef struct {
    char date[11];
    int flight_code;
    double cargo_weight;
    int passengers_count;
    double flight_duration;
    double flight_cost;
    int is_special;
} FlightRecord;

typedef struct {
    FlightRecord *records;
    int count;
    double total_cargo;
    int total_passengers;
} FlightReport;

//
typedef struct {
    char date[11];
    int flight_code;
    double flight_cost;
    int is_special;
    int passengers_count;
    double earnings;
} EarningsRecord;

typedef struct {
    EarningsRecord *records;
    double total_earnings;
    int flight_count;
    int data_exists;
} EarningsReport;

//
typedef struct {
    char date[11];
    int flight_code;
    double flight_cost;
    int is_special;
    int passengers_count;
    double earnings;
    int data_exists;
} EarningsRecordForFlight;

// Структура для хранения полного отчета
typedef struct {
    FlightRecord* records;
    int count;
    int data_exists;
} FlightReport2;

#endif