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
    } crew_members[10]; // предполагаем, что количество членов экипажа не будет превышать 10
    int crew_count;
    struct {
        char date[128];
        int flight_code;
        double flight_cost;
    } flights[10]; // предполагаем, что количество рейсов не будет превышать 10
    int flight_count;
} max_earning_crew_t;

void commander_interface(sqlite3 *db);
void crew_member_interface(sqlite3 *db, int tab_number);

#endif
