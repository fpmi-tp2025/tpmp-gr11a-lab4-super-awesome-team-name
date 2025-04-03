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

#endif