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
