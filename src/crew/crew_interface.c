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