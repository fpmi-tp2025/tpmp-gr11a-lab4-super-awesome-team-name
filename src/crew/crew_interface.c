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

void get_flight_hours_for_crew_helicopter(sqlite3 *db, int tab_number) {
    FlightHoursInfo info = retrieve_flight_hours_data_crew(db, tab_number);

    if (info.data_exists) {
        printf("Вертолет %d\n", info.helicopter_number);
        printf("Общее количество налетанных часов после последнего ремонта: %.2f\n",
               info.total_flight_hours);
        printf("Ресурс летного времени: %d часов\n", info.flight_resource);
        printf("Оставшийся ресурс: %.2f часов\n",
               info.flight_resource - info.total_flight_hours);
    } else {
        printf("Не найдены данные о полетах для члена экипажа с табельным номером %d.\n",
               tab_number);
    }
}

void get_flights_by_period_for_crew(sqlite3 *db, int tab_number) {
    char start_date[11], end_date[11];

    printf("Введите дату начала периода (ГГГГ-ММ-ДД): ");
    scanf("%10s", start_date);
    printf("Введите дату конца периода (ГГГГ-ММ-ДД): ");
    scanf("%10s", end_date);

    if (!validate_date(start_date) || !validate_date(end_date)) {
        printf("Неверный формат даты. Используйте формат ГГГГ-ММ-ДД.\n");
        return;
    }

    FlightReport report = retrieve_flights_report(db, tab_number, start_date, end_date);

    printf("\nРейсы с %s по %s:\n", start_date, end_date);
    printf("Дата\t\tКод\tГруз\tПассажиры\tДлительность\tСтоимость\tТип\n");

    for (int i = 0; i < report.count; i++) {
        FlightRecord *r = &report.records[i];
        printf("%s\t%d\t%.2f\t%d\t\t%.2f\t\t%.2f\t\t%s\n",
               r->date, r->flight_code, r->cargo_weight, r->passengers_count,
               r->flight_duration, r->flight_cost,
               r->is_special ? "Спецрейс" : "Обычный");
    }

    if (report.count > 0) {
        printf("\nСводка:\n");
        printf("Общее количество рейсов: %d\n", report.count);
        printf("Общая масса груза: %.2f кг\n", report.total_cargo);
        printf("Общее количество пассажиров: %d\n", report.total_passengers);
        free(report.records);
    } else {
        printf("Рейсы в указанном периоде не найдены.\n");
    }
}
