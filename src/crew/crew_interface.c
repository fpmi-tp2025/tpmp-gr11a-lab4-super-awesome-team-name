/* crew_interface.c */
#include "../../include/crew/crew_interface.h"
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

void get_crew_member_info(sqlite3 *db, int tab_number) {
    CrewMember* member = get_crew_member_report(db, tab_number);

    if (!member) {
        printf("Член экипажа с табельным номером %d не найден.\n", tab_number);
        return;
    }

    printf("Табельный номер: %d\n", member->tab_number);
    printf("Фамилия: %s\n", member->last_name);
    printf("Должность: %s\n", member->position);
    printf("Стаж: %d лет\n", member->experience_years);
    printf("Адрес: %s\n", member->address);
    printf("Год рождения: %d\n", member->birth_year);
    printf("Номер вертолета: %d\n", member->helicopter_number);

    if (member) {
        free(member->last_name);
        free(member->position);
        free(member->address);
        free(member);
    }
}

void get_helicopter_info(sqlite3 *db, int tab_number) {
    HelicopterInfo info = retrieve_helicopter_info_data(db, tab_number);

    if(info.found) {
        printf("\n=== Helicopter Information ===\n"
               "Number:    %d\n"
               "Model:     %s\n"
               "Manufacture date: %s\n"
               "Payload:   %.2f kg\n"
               "Last Repair: %s\n"
               "Resource:  %d hours\n",
               info.helicopter_number,
               info.model,
               info.manufacture_date,
               info.max_payload,
               info.last_repair_date,
               info.flight_resource);
    } else {
        printf("No helicopter found for crew member #%d\n", tab_number);
    }

    // Освобождение памяти
    free(info.model);
    free(info.manufacture_date);
    free(info.last_repair_date);

    // Сброс указателей
    info.model = NULL;
    info.manufacture_date = NULL;
    info.last_repair_date = NULL;
}
