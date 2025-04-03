#ifndef CREW_INTERFACE_H
#define CREW_INTERFACE_H

#include <sqlite3.h>
#include "models/crew_struct.h"
#include "crew.h"

void get_crew_member_info(sqlite3 *db, int tab_number);

#endif /* CREW_INTERFACE_H */