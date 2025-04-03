/* interface.h */
#ifndef INTERFACE_H
#define INTERFACE_H

#include <sqlite3.h>
#include "crew.h"
#include "commander.h"

void commander_interface(sqlite3 *db);
void crew_member_interface(sqlite3 *db, int tab_number);

#endif
