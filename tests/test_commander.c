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
            "helicopter_number INTEGER, "
            "flight_cost REAL, "
            "cargo_weight REAL, "
            "passengers_count INTEGER, "
            "date TEXT);";  // Объединенная схема для обоих тестов

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
START_TEST(test_get_max_earning_crew) {
        sqlite3 *db = connect_db(":memory:");
        initialize_db(db);

        // Вставка тестовых данных
        sqlite3_exec(db,
        "INSERT INTO Helicopter VALUES (101, 'Model X');"
        "INSERT INTO Crew_member VALUES (1234, 'John Doe', 101);"
        "INSERT INTO Flight VALUES (2001, 101, 1000.50, NULL, NULL, '2025-04-01');",
        NULL, NULL, NULL);

        max_earning_crew_t result = get_max_earning_crew(db);

        ck_assert_int_eq(result.helicopter_number, 101);
        ck_assert_str_eq(result.helicopter_model, "Model X");
        ck_assert_double_eq(result.total_earnings, 1000.50);
        ck_assert_int_eq(result.crew_count, 1);
        ck_assert_int_eq(result.crew_members[0].tab_number, 1234);

        sqlite3_close(db);
}
END_TEST

// Тест для get_flights_report
START_TEST(test_get_flights_report) {
    sqlite3 *db = connect_db(":memory:");
    initialize_db(db);

    sqlite3_exec(db,
                 "INSERT INTO Helicopter VALUES (101, 'Model X');"
                 "INSERT INTO Flight VALUES (2001, 101, NULL, 100.5, 5, '2024-04-01');"
                 "INSERT INTO Flight VALUES (2002, 101, NULL, 200.0, 10, '2024-04-02');",
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

// Создание тестового набора
Suite* commander_suite(void) {
    Suite *s = suite_create("Commander");
    TCase *tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_get_max_earning_crew);
    tcase_add_test(tc_core, test_get_flights_report);
    tcase_add_test(tc_core, test_invalid_dates);

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