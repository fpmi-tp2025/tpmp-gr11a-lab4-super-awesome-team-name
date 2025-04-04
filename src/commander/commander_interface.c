/* commander_interface.c */
#include "../../include/commander/commander_interface.h"
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

void get_flights_data_by_period(sqlite3 *db) {
    char start[11], end[11];

    printf("Введите начальную дату (YYYY-MM-DD): ");
    scanf("%10s", start);
    printf("Введите конечную дату (YYYY-MM-DD): ");
    scanf("%10s", end);

    FlightPeriodReport *report = get_flights_report(db, start, end);
    if (!report) return;

    // Вывод результатов
    for (FlightPeriodReport *r = report; r != NULL; r = r->next) {
        printf("\nВертолет №%d\n", r->helicopter_number);
        for (int i = 0; i < r->total_flights; i++) {
            printf("Рейс %d: Груз %.2f кг, %d пассажиров\n",
                   r->flight_codes[i],
                   r->cargo_weights[i],
                   r->passengers[i]);
        }
        printf("Итого: %.2f кг, %d человек\n",
               r->total_cargo,
               r->total_passengers);
    }

    // Очистка памяти
    while (report) {
        FlightPeriodReport *tmp = report;
        report = report->next;
        free(tmp->flight_codes);
        free(tmp->cargo_weights);
        free(tmp->passengers);
        free(tmp);
    }
}

void get_flights_hours_after_repair(sqlite3 *db) {
    int result_count = 0;
    HelicopterHours *data = retrieve_flight_hours_data(db, &result_count);

    if (!data) {
        printf("Данные не найдены или произошла ошибка\n\n");
        return;
    }

    for (int i = 0; i < result_count; i++) {
        printf("Helicopter %d\n", data[i].helicopter_number);
        printf("Налетанные часы после капитального ремонта: %.2f\n", data[i].total_flight_hours);
        printf("Ресурс летного времени: %d\n\n", data[i].flight_resource);
    }

    free(data);
}

void get_special_flights_summary(sqlite3 *db) {
    int result_count = 0;
    SpecialFlightsSummary *data = retrieve_special_flights_data(db, &result_count);

    if (!data) {
        printf("Нет данных о спецрейсах или произошла ошибка\n\n");
        return;
    }

    for (int i = 0; i < result_count; i++) {
        printf("Helicopter %d\n", data[i].helicopter_number);
        printf("Количество спецрейсов: %d\n", data[i].flights_count);
        printf("Общая масса перевезенных грузов: %.2f\n", data[i].total_cargo_weight);
        printf("Общая сумма заработанных денег: %.2f$\n\n", data[i].total_income);
    }

    free(data);
}

void get_max_earning_crew(sqlite3 *db) {
    max_earning_crew_t data = retrieve_max_earning_crew_data(db);

    if (data.helicopter_number == 0) {
        printf("Данные не найдены\n");
    }

    // Форматированный вывод
    printf("\nВертолет с максимальными заработками:\n");
    printf("Номер: %d\nМодель: %s\nЗаработок: %.2f$\n",
           data.helicopter_number,
           data.helicopter_model,
           data.total_earnings);

    printf("\nСостав экипажа (%d чел.):\n", data.crew_count);
    for (int i = 0; i < data.crew_count; i++) {
        printf("%d. %s (таб. №%d)\n",
               i+1,
               data.crew_members[i].last_name,
               data.crew_members[i].tab_number);
    }

    printf("\nПоследние рейсы (%d шт.):\n", data.flight_count);
    for (int i = 0; i < data.flight_count; i++) {
        printf("[%s] Рейс %d: %.2f$\n",
               data.flights[i].date,
               data.flights[i].flight_code,
               data.flights[i].flight_cost);
    }
}