#ifndef COMMANDER_INTERFACE_H
#define COMMANDER_INTERFACE_H
#include <sqlite3.h>
#include "models/commander_struct.h"
#include "commander.h"

void get_flights_data_by_period(sqlite3 *db);
void get_flights_hours_after_repair(sqlite3 *db);
void get_special_flights_summary(sqlite3 *db);

#endif /* COMMANDER_INTERFACE_H */