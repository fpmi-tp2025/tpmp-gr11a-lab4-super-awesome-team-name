#include <check.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include "validation.h"

// Тестируемая функция
int validate_flight_code(sqlite3 *db, int flight_code);

// Функция для подключения к базе данных
sqlite3* connect_db(const char *db_name) {
    sqlite3 *db;
    if (sqlite3_open(db_name, &db) != SQLITE_OK) {
        fprintf(stderr, "Не удалось открыть базу данных: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

// Функция для инициализации базы данных (создание таблицы)
void initialize_db(sqlite3* db) {
    const char *sql = "CREATE TABLE IF NOT EXISTS Flight (flight_code INTEGER PRIMARY KEY);";
    char *err_msg = NULL;
    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Ошибка при инициализации базы данных: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

// Функция для отключения от базы данных
void disconnect_db(sqlite3* db) {
    sqlite3_close(db);
}

// Тест для функции validate_flight_code
START_TEST(test_validate_flight_code) {
        sqlite3 *db = connect_db(":memory:");
        initialize_db(db);

        // Вставляем несколько тестовых записей в таблицу Flight
        const char *insert_sql = "INSERT INTO Flight (flight_code) VALUES (?);";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0);

        // Вставляем данные для теста
        sqlite3_bind_int(stmt, 1, 1001);
        sqlite3_step(stmt);
        sqlite3_clear_bindings(stmt);

        sqlite3_bind_int(stmt, 1, 1002);
        sqlite3_step(stmt);

        sqlite3_finalize(stmt);

        // Проверка: существование flight_code 1001
        ck_assert(validate_flight_code(db, 1001) == 1);  // Ожидаем TRUE (существует)

        // Проверка: несуществующий flight_code
        ck_assert(validate_flight_code(db, 9999) == 0);  // Ожидаем FALSE (не существует)

        disconnect_db(db);  // Отключаемся от базы данных
}
END_TEST

// Создание тестового набора
Suite* flight_suite(void) {
    Suite* s;
    TCase* tc_core;

    s = suite_create("Flight");

    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_validate_flight_code);  // Добавляем тест
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    int number_failed;
    Suite* s;
    SRunner* sr;

    s = flight_suite();  // Получаем набор тестов
    sr = srunner_create(s);  // Создаем объект для запуска тестов

    srunner_run_all(sr, CK_NORMAL);  // Запуск всех тестов
    number_failed = srunner_ntests_failed(sr);  // Получаем количество неудачных тестов
    srunner_free(sr);  // Освобождаем ресурсы

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;  // Возвращаем статус завершения
}
