#include <check.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
#include "crew.h"

// Функция для подключения к базе данных
sqlite3* connect_db(const char *db_name) {
    sqlite3 *db;
    if (sqlite3_open(db_name, &db) != SQLITE_OK) {
        fprintf(stderr, "Не удалось открыть базу данных: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

// Функция для инициализации базы данных
void initialize_db(sqlite3* db) {
    const char *sql =
            "CREATE TABLE IF NOT EXISTS Helicopter (helicopter_number INTEGER PRIMARY KEY, model TEXT, "
            "manufacture_date TEXT, max_payload REAL, last_repair_date TEXT, flight_resource INTEGER);"
            "CREATE TABLE IF NOT EXISTS Crew_member (tab_number INTEGER PRIMARY KEY, last_name TEXT, helicopter_number INTEGER);";
    char *err_msg = NULL;
    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Ошибка при инициализации базы данных: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

// Тест для функции get_helicopter_info
START_TEST(test_get_helicopter_info) {
        sqlite3 *db = connect_db(":memory:");
        initialize_db(db);

        // Вставляем тестовые данные
        const char *insert_sql_heli = "INSERT INTO Helicopter (helicopter_number, model, manufacture_date, max_payload, last_repair_date, flight_resource) VALUES (?, ?, ?, ?, ?, ?);";
        sqlite3_stmt *stmt_heli;
        sqlite3_prepare_v2(db, insert_sql_heli, -1, &stmt_heli, 0);

        sqlite3_bind_int(stmt_heli, 1, 101);
        sqlite3_bind_text(stmt_heli, 2, "Model X", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_heli, 3, "2020-01-01", -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt_heli, 4, 1500.50);
        sqlite3_bind_text(stmt_heli, 5, "2025-03-01", -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt_heli, 6, 500);
        sqlite3_step(stmt_heli);
        sqlite3_finalize(stmt_heli);

        const char *insert_sql_crew = "INSERT INTO Crew_member (tab_number, last_name, helicopter_number) VALUES (?, ?, ?);";
        sqlite3_stmt *stmt_crew;
        sqlite3_prepare_v2(db, insert_sql_crew, -1, &stmt_crew, 0);

        sqlite3_bind_int(stmt_crew, 1, 1234);
        sqlite3_bind_text(stmt_crew, 2, "John Doe", -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt_crew, 3, 101);
        sqlite3_step(stmt_crew);
        sqlite3_finalize(stmt_crew);

        // Выполняем тест
        HelicopterInfo result = get_helicopter_info(db, 1234);

        // Проверяем результаты
        ck_assert_int_eq(result.helicopter_number, 101);
        ck_assert_str_eq(result.model, "Model X");
        ck_assert_str_eq(result.manufacture_date, "2020-01-01");
        ck_assert_double_eq(result.max_payload, 1500.50);
        ck_assert_str_eq(result.last_repair_date, "2025-03-01");
        ck_assert_int_eq(result.flight_resource, 500);
        ck_assert_int_eq(result.found, 1);

        sqlite3_close(db);
}
END_TEST

// Создание тестового набора
Suite* helicopter_suite(void) {
    Suite* s;
    TCase* tc_core;

    s = suite_create("Helicopter");

    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_get_helicopter_info);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    int number_failed;
    Suite* s;
    SRunner* sr;

    s = helicopter_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
