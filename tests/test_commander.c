#include <check.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
#include "../include/commander/commander.h"

// Общие функции для работы с БД
sqlite3* connect_db(const char *db_name) {
    sqlite3 *db;
    if (sqlite3_open(db_name, &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

void initialize_db(sqlite3* db) {
    const char *sql =
            "CREATE TABLE IF NOT EXISTS Helicopter ("
            "helicopter_number INTEGER PRIMARY KEY, "
            "model TEXT);"
            "CREATE TABLE IF NOT EXISTS Crew_member ("
            "tab_number INTEGER PRIMARY KEY, "
            "last_name TEXT, "
            "helicopter_number INTEGER);"
            "CREATE TABLE IF NOT EXISTS Flight ("
            "flight_code INTEGER PRIMARY KEY, "
            "helicopter_number INTEGER NOT NULL, "
            "flight_cost REAL DEFAULT 0.0, "
            "cargo_weight REAL DEFAULT 0.0, "
            "passengers_count INTEGER DEFAULT 0, "
            "date TEXT NOT NULL, "
            "is_special INTEGER NOT NULL DEFAULT 0);";  // Исправленная структура

    char *err_msg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

// Функция для освобождения памяти отчета
void free_report(FlightPeriodReport *report) {
    while (report != NULL) {
        FlightPeriodReport *next = report->next;
        free(report->flight_codes);
        free(report->cargo_weights);
        free(report->passengers);
        free(report);
        report = next;
    }
}

// Тест для get_max_earning_crew
START_TEST(test_retrieve_max_earning_crew_data) {
        sqlite3 *db = connect_db(":memory:");
        initialize_db(db);

        // Вставка данных с явным указанием столбцов
        sqlite3_exec(db,
        "INSERT INTO Helicopter VALUES (101, 'Model X');"
        "INSERT INTO Crew_member VALUES (1234, 'John Doe', 101);"
        "INSERT INTO Flight (flight_code, helicopter_number, flight_cost, date, is_special) "
        "VALUES (2001, 101, 1000.50, '2025-04-01', 0);",  // Явное указание is_special
        NULL, NULL, NULL);

        max_earning_crew_t result = retrieve_max_earning_crew_data(db);

        ck_assert_int_eq(result.helicopter_number, 101);
        ck_assert_str_eq(result.helicopter_model, "Model X");
        ck_assert_double_eq_tol(result.total_earnings, 1000.50, 0.01);
        ck_assert_int_eq(result.crew_count, 1);
        ck_assert_int_eq(result.crew_members[0].tab_number, 1234);

        sqlite3_close(db);
}
END_TEST

// Тест для get_flights_report
START_TEST(test_get_flights_report) {
    sqlite3 *db = connect_db(":memory:");
    initialize_db(db);

    // Вставка данных с явным указанием столбцов
    sqlite3_exec(db,
                 "INSERT INTO Helicopter VALUES (101, 'Model X');"
                 "INSERT INTO Flight (flight_code, helicopter_number, cargo_weight, passengers_count, date, is_special) "
                 "VALUES (2001, 101, 100.5, 5, '2024-04-01', 0), "
                 "(2002, 101, 200.0, 10, '2024-04-02', 0);",
                 NULL, NULL, NULL);

    FlightPeriodReport *report = get_flights_report(db, "2024-04-01", "2024-04-02");

    ck_assert_ptr_nonnull(report);
    ck_assert_int_eq(report->helicopter_number, 101);
    ck_assert_int_eq(report->total_flights, 2);
    ck_assert_double_eq_tol(report->total_cargo, 300.5, 0.001);

    free_report(report);
    sqlite3_close(db);
}
END_TEST

// Тест на невалидные даты
START_TEST(test_invalid_dates) {
    sqlite3 *db = connect_db(":memory:");
    FlightPeriodReport *report = get_flights_report(db, "invalid-date", "2024-04-02");
    ck_assert_ptr_null(report);
    sqlite3_close(db);
}
END_TEST

START_TEST(test_retrieve_pilot_earnings_by_flights) {
    sqlite3 *db = connect_db(":memory:");
    initialize_db(db);

    // Вставка данных с явным указанием столбцов
    sqlite3_exec(db,
                 "INSERT INTO Crew_member VALUES (777, 'Петров', 101);"
                 "INSERT INTO Flight (flight_code, helicopter_number, flight_cost, passengers_count, date, is_special) "
                 "VALUES (1001, 101, 30000.0, 5, '2024-01-01', 1), "  // Специальный рейс
                 "(1002, 101, 20000.0, 3, '2024-01-02', 0), "  // Обычный рейс
                 "(1003, 101, 50000.0, 4, '2024-02-01', 1), "  // Вне диапазона
                 "(1004, 101, 15000.0, 2, '2024-01-03', 1);",  // Специальный рейс
                 NULL, NULL, NULL);

    int flights_count = 0;

    // Тест 1: Все рейсы за январь
    DetailedPilotEarnings earnings = retrieve_pilot_earnings_by_flights(
            db, 777, "2024-01-01", "2024-01-31", -1, &flights_count
    );

    ck_assert_int_eq(earnings.pilot_id, 777);
    ck_assert_str_eq(earnings.pilot_name, "Петров");
    ck_assert_int_eq(flights_count, 3);
    ck_assert_double_eq_tol(earnings.total_earnings,
                            (30000*0.10) + (20000*0.05) + (15000*0.10), 0.01);

    // Проверка первого рейса
    ck_assert_int_eq(earnings.flights[0].flight_code, 1001);
    ck_assert_str_eq(earnings.flights[0].flight_date, "2024-01-01");
    ck_assert_int_eq(earnings.flights[0].is_special, 1);
    ck_assert_double_eq(earnings.flights[0].flight_cost, 30000.0);

    free(earnings.pilot_name);
    free(earnings.flights);

    sqlite3_close(db);
}
END_TEST

// Создание тестового набора
Suite* commander_suite(void) {
    Suite *s = suite_create("Commander");
    TCase *tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_retrieve_max_earning_crew_data);
    tcase_add_test(tc_core, test_get_flights_report);
    tcase_add_test(tc_core, test_invalid_dates);
    tcase_add_test(tc_core, test_retrieve_pilot_earnings_by_flights);

    suite_add_tcase(s, tc_core);
    return s;
}

int main(void) {
    int number_failed;
    Suite *s = commander_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}