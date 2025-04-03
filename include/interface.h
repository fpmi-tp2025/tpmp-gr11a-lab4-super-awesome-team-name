#ifndef TEAM_LAB4_INTERFACE_H
#define TEAM_LAB4_INTERFACE_H
#include <sqlite3.h>
#include "commander/commander_interface.h"
#include "crew/crew_interface.h"

void commander_interface(sqlite3 *db);
void crew_member_interface(sqlite3 *db, int tab_number);

#endif //TEAM_LAB4_INTERFACE_H
