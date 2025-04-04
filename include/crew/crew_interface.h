/* crew_interface.h */
#ifndef CREW_INTERFACE_H
#define CREW_INTERFACE_H

#include <sqlite3.h>
#include "models/crew_struct.h"
#include "crew.h"
#include "../validation.h"

void get_crew_member_info(sqlite3 *db, int tab_number);
void get_helicopter_info(sqlite3 *db, int tab_number);
void get_flight_hours_for_crew_helicopter(sqlite3 *db, int tab_number);
void get_flights_by_period_for_crew(sqlite3 *db, int tab_number);

#endif /* CREW_INTERFACE_H */