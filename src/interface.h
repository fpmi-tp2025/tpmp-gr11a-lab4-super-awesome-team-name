/* interface.h */
#ifndef INTERFACE_H
#define INTERFACE_H

#include <sqlite3.h>

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
    } flights[100];
    int flight_count;
} max_earning_crew_t;

typedef struct {
    int helicopter_number;
    const char *model;
    const char *manufacture_date;
    double max_payload;
    const char *last_repair_date;
    int flight_resource;
    int found;
} HelicopterInfo;

void commander_interface(sqlite3 *db);
void crew_member_interface(sqlite3 *db, int tab_number);

#endif
