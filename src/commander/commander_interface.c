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

    if (!validate_date(start) || !validate_date(end)) {
        printf("Некорректный формат дат\n");
    }

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

void get_helicopter_with_most_flights(sqlite3 *db) {
    // Вызываем логику
    HelicopterWithCrewData *data = get_helicopter_with_crew_most_flights(db);
    
    if (!data) {
        printf("Не удалось найти вертолет с максимальным количеством рейсов.\n");
        return;
    }
    
    // Выводим информацию о вертолете
    printf("Номер вертолета: %d\n", data->helicopter.helicopter_number);
    printf("Модель вертолета: %s\n", data->helicopter.helicopter_model);
    printf("Количество рейсов: %d\n", data->helicopter.num_flights);
    printf("Количество заработанных денег: %.2f$\n", data->helicopter.total_earnings);
    
    // Выводим информацию о экипаже
    printf("Экипаж:\n");
    for (int i = 0; i < data->crew_count; i++) {
        printf("Табельный номер: %d, Фамилия: %s\n", 
               data->crew_members[i].tab_number, 
               data->crew_members[i].last_name);
    }
    
    // Освобождаем память
    if (data) {
        free((void*)data->helicopter.helicopter_model);
        
        for (int i = 0; i < data->crew_count; i++) {
            free((void*)data->crew_members[i].last_name);
        }
        
        free(data->crew_members);
        free(data);
    }
}

void get_normal_flights_summary(sqlite3 *db) {
    int count = 0;
    HelicopterSummary* data = retrieve_normal_flights_data(db, &count);

    if (!data) {
        printf("Данные по обычным рейсам не найдены\n");
        return;
    }

    printf("=== Сводка по обычным рейсам ===\n");
    for (int i = 0; i < count; i++) {
        printf("Вертолет %d\n", data[i].helicopter_number);
        printf("Модель: %s\n", data[i].model);
        printf("Количество рейсов: %d\n", data[i].num_flights);
        printf("Общий вес грузов: %.2f кг\n", data[i].total_cargo_weight);
        printf("Суммарный заработок: %.2f $\n\n", data[i].total_earnings);

        free(data[i].model);
    }

    free(data);
}

void update_crew_member(sqlite3 *db) {
    int tab_number;
    char field[50];
    char new_value[100];

    // Ввод данных
    printf("Табельный номер: ");
    if (scanf("%d", &tab_number) != 1) {
        printf("Ошибка ввода номера\n");
        while(getchar() != '\n'); // Очистка буфера
        return;
    }

    printf("Поле для изменения (last_name/position/birth_year/address/helicopter_number): ");
    if (scanf("%49s", field) != 1) {
        printf("Ошибка ввода поля\n");
        return;
    }

    printf("Новое значение: ");
    if (scanf("%99s", new_value) != 1) {
        printf("Ошибка ввода значения\n");
        return;
    }

    // Вызов логической части
    int success = update_crew_member_db(db, tab_number, field, new_value);

    // Вывод результата
    if (success) {
        printf("Данные успешно обновлены\n");
    } else {
        printf("Ошибка обновления. Проверьте введенные данные\n");
    }
}

//SELECT
void get_pilot_earnings_by_period(sqlite3 *db) {
    int pilot_id;
    char start_date[11], end_date[11]; // Формат YYYY-MM-DD
    
    // Получение входных данных от пользователя
    printf("Введите табельный номер летчика: ");
    scanf("%d", &pilot_id);
    
    printf("Введите начальную дату (ГГГГ-ММ-ДД): ");
    scanf("%10s", start_date);
    
    printf("Введите конечную дату (ГГГГ-ММ-ДД): ");
    scanf("%10s", end_date);
    
    // Вызов логики для получения данных
    PilotEarnings earnings = retrieve_pilot_earnings(db, pilot_id, start_date, end_date);
    
    // Вывод результатов
    if (earnings.pilot_name == NULL) {
        printf("Данные о доходах летчика с табельным номером %d за указанный период не найдены\n", pilot_id);
        return;
    }
    
    printf("=== Данные о доходах летчика ===\n");
    printf("Табельный номер: %d\n", earnings.pilot_id);
    printf("Фамилия: %s\n", earnings.pilot_name);
    printf("Период: с %s по %s\n", start_date, end_date);
    printf("Количество рейсов: %d\n", earnings.flight_count);
    printf("Общий доход: %.2f $\n", earnings.total_earnings);
    
    // Освобождение памяти
    free(earnings.pilot_name);
}

void get_pilot_earnings_by_flights(sqlite3 *db) {
    int pilot_id;
    int flight_type = -1; // -1: все рейсы, 0: обычные, 1: спецрейсы
    char start_date[11], end_date[11]; // Формат YYYY-MM-DD
    int flights_count = 0;
    
    // Получение входных данных от пользователя
    printf("Введите табельный номер летчика: ");
    scanf("%d", &pilot_id);
    
    printf("Введите начальную дату (ГГГГ-ММ-ДД): ");
    scanf("%10s", start_date);
    
    printf("Введите конечную дату (ГГГГ-ММ-ДД): ");
    scanf("%10s", end_date);
    
    printf("Выберите тип рейсов (0 - обычные, 1 - спецрейсы, -1 - все): ");
    scanf("%d", &flight_type);
    
    // Вызов логики для получения данных
    DetailedPilotEarnings earnings = retrieve_pilot_earnings_by_flights(db, pilot_id, start_date, end_date, flight_type, &flights_count);
    
    // Вывод результатов
    if (earnings.pilot_name == NULL) {
        printf("Данные о доходах летчика с табельным номером %d за указанный период не найдены\n", pilot_id);
        return;
    }
    
    // Определение типа рейсов для вывода
    char* flight_type_str;
    if (flight_type == -1) flight_type_str = "всех";
    else if (flight_type == 0) flight_type_str = "обычных";
    else flight_type_str = "специальных";
    
    printf("=== Данные о доходах летчика за %s рейсы ===\n", flight_type_str);
    printf("Табельный номер: %d\n", earnings.pilot_id);
    printf("Фамилия: %s\n", earnings.pilot_name);
    printf("Период: с %s по %s\n", start_date, end_date);
    printf("Количество рейсов: %d\n", earnings.flight_count);
    printf("Общий доход: %.2f $\n\n", earnings.total_earnings);
    
    // Детальная информация по каждому рейсу
    printf("Детализация по рейсам:\n");
    printf("%-12s %-12s %-15s %-15s %-15s\n", "Код рейса", "Дата", "Тип", "Стоимость($)", "Доход пилота($)");
    printf("-------------------------------------------------------------------------\n");
    
    for (int i = 0; i < flights_count; i++) {
        printf("%-12d %-12s %-15s %-15.2f %-15.2f\n", 
               earnings.flights[i].flight_code,
               earnings.flights[i].flight_date,
               earnings.flights[i].is_special ? "Спецрейс" : "Обычный",
               earnings.flights[i].flight_cost,
               earnings.flights[i].pilot_earnings);
               
        free(earnings.flights[i].flight_date); // Освобождение памяти для даты
    }
    
    // Освобождение памяти
    free(earnings.pilot_name);
    free(earnings.flights);
}