/* crew.c */
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/crew/crew.h"
#include "../../include/validation.h"

// Функция для получения информации о члене экипажа по его табельному номеру
CrewMember* get_crew_member_report(sqlite3 *db, int tab_number) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT tab_number, last_name, position, experience_years, address, birth_year, helicopter_number "
                        "FROM Crew_member "
                        "WHERE tab_number = ?";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    sqlite3_bind_int(stmt, 1, tab_number);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    // Выделяем память для структуры
    CrewMember* member = (CrewMember*)malloc(sizeof(CrewMember));
    if (!member) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    // Заполняем поля структуры
    member->tab_number = sqlite3_column_int(stmt, 0);

    member->last_name = strdup((const char*)sqlite3_column_text(stmt, 1));
    member->position = strdup((const char*)sqlite3_column_text(stmt, 2));
    member->experience_years = sqlite3_column_int(stmt, 3);
    member->address = strdup((const char*)sqlite3_column_text(stmt, 4));
    member->birth_year = sqlite3_column_int(stmt, 5);
    member->helicopter_number = sqlite3_column_int(stmt, 6);

    sqlite3_finalize(stmt);
    return member;
}

HelicopterInfo retrieve_helicopter_info_data(sqlite3 *db, int tab_number) {
    sqlite3_stmt *stmt;
    HelicopterInfo info = {0};

    const char *query =
            "SELECT h.helicopter_number, h.model, h.manufacture_date, "
            "h.max_payload, h.last_repair_date, h.flight_resource "
            "FROM Helicopter h "
            "JOIN Crew_member cm ON h.helicopter_number = cm.helicopter_number "
            "WHERE cm.tab_number = ?";

    // Подготовка запроса
    if(sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
        return info;
    }

    sqlite3_bind_int(stmt, 1, tab_number);

    // Обработка результатов
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        info.helicopter_number = sqlite3_column_int(stmt, 0);
        info.found = 1;

        // Обработка строковых полей
        const char *text;

        text = (const char*)sqlite3_column_text(stmt, 1);
        info.model = text ? strdup(text) : strdup("N/A");

        text = (const char*)sqlite3_column_text(stmt, 2);
        info.manufacture_date = text ? strdup(text) : strdup("N/A");

        info.max_payload = sqlite3_column_double(stmt, 3);

        text = (const char*)sqlite3_column_text(stmt, 4);
        info.last_repair_date = text ? strdup(text) : strdup("N/A");

        info.flight_resource = sqlite3_column_int(stmt, 5);
    }

    sqlite3_finalize(stmt);
    return info;
}

// Функция для получения налетанных часов и оставшегося ресурса вертолета члена экипажа
FlightHoursInfo retrieve_flight_hours_data_crew(sqlite3 *db, int tab_number) {
    sqlite3_stmt *stmt;
    FlightHoursInfo result = {0};
    const char *query =
            "SELECT h.helicopter_number, SUM(f.flight_duration), h.flight_resource "
            "FROM Flight f "
            "JOIN Helicopter h ON f.helicopter_number = h.helicopter_number "
            "JOIN Crew_member cm ON h.helicopter_number = cm.helicopter_number "
            "WHERE cm.tab_number = ? AND f.date > h.last_repair_date "
            "GROUP BY h.helicopter_number";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return result;
    }

    sqlite3_bind_int(stmt, 1, tab_number);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result.helicopter_number = sqlite3_column_int(stmt, 0);
        result.total_flight_hours = sqlite3_column_double(stmt, 1);
        result.flight_resource = sqlite3_column_int(stmt, 2);
        result.data_exists = 1;
    }

    sqlite3_finalize(stmt);
    return result;
}

// Функция для получения данных о рейсах вертолета члена экипажа за указанный период
FlightReport retrieve_flights_report(sqlite3 *db, int tab_number, const char *start_date, const char *end_date) {
    sqlite3_stmt *stmt;
    FlightReport report = {0};
    const char *query =
            "SELECT f.date, f.flight_code, f.cargo_weight, f.passengers_count, "
            "f.flight_duration, f.flight_cost, f.is_special "
            "FROM Flight f "
            "JOIN Crew_member cm ON f.helicopter_number = cm.helicopter_number "
            "WHERE cm.tab_number = ? AND f.date BETWEEN ? AND ? "
            "ORDER BY f.date";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return report;
    }

    sqlite3_bind_int(stmt, 1, tab_number);
    sqlite3_bind_text(stmt, 2, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, end_date, -1, SQLITE_STATIC);

    // Первый проход для подсчета записей
    while (sqlite3_step(stmt) == SQLITE_ROW) report.count++;
    sqlite3_reset(stmt);

    if (report.count == 0) {
        sqlite3_finalize(stmt);
        return report;
    }

    report.records = malloc(sizeof(FlightRecord) * report.count);
    if (!report.records) {
        sqlite3_finalize(stmt);
        report.count = 0;
        return report;
    }

    // Второй проход для заполнения данных
    for (int i = 0; i < report.count; i++) {
        sqlite3_step(stmt);

        strncpy(report.records[i].date, (const char*)sqlite3_column_text(stmt, 0), 10);
        report.records[i].flight_code = sqlite3_column_int(stmt, 1);
        report.records[i].cargo_weight = sqlite3_column_double(stmt, 2);
        report.records[i].passengers_count = sqlite3_column_int(stmt, 3);
        report.records[i].flight_duration = sqlite3_column_double(stmt, 4);
        report.records[i].flight_cost = sqlite3_column_double(stmt, 5);
        report.records[i].is_special = sqlite3_column_int(stmt, 6);

        report.total_cargo += report.records[i].cargo_weight;
        report.total_passengers += report.records[i].passengers_count;
    }

    sqlite3_finalize(stmt);
    return report;
}


// Функция для расчета денег, заработанных членом экипажа за определенный период
EarningsReport retrieve_earnings_data(sqlite3 *db, int tab_number, const char *start_date, const char *end_date) {
    sqlite3_stmt *stmt;
    EarningsReport report = {0};
    const char *query =
            "SELECT f.date, f.flight_code, f.flight_cost, f.is_special, f.passengers_count "
            "FROM Flight f "
            "JOIN Crew_member cm ON f.helicopter_number = cm.helicopter_number "
            "WHERE cm.tab_number = ? AND f.date BETWEEN ? AND ? "
            "ORDER BY f.date";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return report;
    }

    sqlite3_bind_int(stmt, 1, tab_number);
    sqlite3_bind_text(stmt, 2, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, end_date, -1, SQLITE_STATIC);

    // Подсчет количества записей
    while (sqlite3_step(stmt) == SQLITE_ROW) report.flight_count++;
    sqlite3_reset(stmt);

    if (report.flight_count == 0) {
        sqlite3_finalize(stmt);
        return report;
    }

    // Выделение памяти
    report.records = malloc(sizeof(EarningsRecord) * report.flight_count);
    if (!report.records) {
        sqlite3_finalize(stmt);
        report.flight_count = 0;
        return report;
    }

    // Заполнение данных
    for (int i = 0; i < report.flight_count; i++) {
        sqlite3_step(stmt);

        strncpy(report.records[i].date, (const char*)sqlite3_column_text(stmt, 0), 10);
        report.records[i].flight_code = sqlite3_column_int(stmt, 1);
        report.records[i].flight_cost = sqlite3_column_double(stmt, 2);
        report.records[i].is_special = sqlite3_column_int(stmt, 3);
        report.records[i].passengers_count = sqlite3_column_int(stmt, 4);

        // Расчет заработка
        double percentage = report.records[i].is_special ? 0.10 : 0.05;
        report.records[i].earnings = (report.records[i].flight_cost * percentage *
                                      report.records[i].passengers_count) / 3;
        report.total_earnings += report.records[i].earnings;
    }

    report.data_exists = 1;
    sqlite3_finalize(stmt);
    return report;
}

// Функция для расчета денег, заработанных членом экипажа за конкретный рейс
EarningsRecordForFlight retrieve_earnings_data_for_flight(sqlite3 *db, int tab_number, int flight_code) {
    sqlite3_stmt *stmt;
    EarningsRecordForFlight record = {0};
    const char *query =
            "SELECT f.date, f.flight_cost, f.is_special, f.passengers_count "
            "FROM Flight f "
            "JOIN Crew_member cm ON f.helicopter_number = cm.helicopter_number "
            "WHERE cm.tab_number = ? AND f.flight_code = ?";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return record;
    }

    sqlite3_bind_int(stmt, 1, tab_number);
    sqlite3_bind_int(stmt, 2, flight_code);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Заполнение данных
        strncpy(record.date, (const char*)sqlite3_column_text(stmt, 0), sizeof(record.date)-1);
        record.flight_code = flight_code;
        record.flight_cost = sqlite3_column_double(stmt, 1);
        record.is_special = sqlite3_column_int(stmt, 2);
        record.passengers_count = sqlite3_column_int(stmt, 3);

        // Расчет заработка
        double percentage = record.is_special ? 0.10 : 0.05;
        record.earnings = (record.flight_cost * percentage * record.passengers_count) / 3;
        record.data_exists = 1;
    }

    sqlite3_finalize(stmt);
    return record;
}

// Функция для получения информации о всех рейсах члена экипажа
FlightReport2 retrieve_all_flights_data(sqlite3* db, int tab_number) {
    sqlite3_stmt* stmt;
    FlightReport2 report = {0};
    const char* query =
            "SELECT f.date, f.flight_code, f.cargo_weight, f.passengers_count, "
            "f.flight_duration, f.flight_cost, f.is_special "
            "FROM Flight f "
            "JOIN Crew_member cm ON f.helicopter_number = cm.helicopter_number "
            "WHERE cm.tab_number = ? "
            "ORDER BY f.date";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return report;
    }

    sqlite3_bind_int(stmt, 1, tab_number);

    // Первый проход для подсчета записей
    while (sqlite3_step(stmt) == SQLITE_ROW) report.count++;
    sqlite3_reset(stmt);

    if (report.count == 0) {
        sqlite3_finalize(stmt);
        return report;
    }

    // Выделение памяти
    report.records = malloc(sizeof(FlightRecord) * report.count);
    if (!report.records) {
        sqlite3_finalize(stmt);
        report.count = 0;
        return report;
    }

    // Заполнение данных
    for (int i = 0; i < report.count; i++) {
        sqlite3_step(stmt);

        // Обработка строковых полей
        strncpy(report.records[i].date, (const char*)sqlite3_column_text(stmt, 0), 10);
        report.records[i].flight_code = sqlite3_column_int(stmt, 1);
        report.records[i].cargo_weight = sqlite3_column_double(stmt, 2);
        report.records[i].passengers_count = sqlite3_column_int(stmt, 3);
        report.records[i].flight_duration = sqlite3_column_double(stmt, 4);
        report.records[i].flight_cost = sqlite3_column_double(stmt, 5);
        report.records[i].is_special = sqlite3_column_int(stmt, 6);
    }

    report.data_exists = 1;
    sqlite3_finalize(stmt);
    return report;
}

// Логическая часть: обновление адреса
OperationStatus update_crew_address(sqlite3 *db, const UpdateData *data) {
    sqlite3_stmt *stmt;
    OperationStatus status = {0};

    const char *query = "UPDATE Crew_member SET address = ? WHERE tab_number = ?";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        status.error_message = sqlite3_errmsg(db);
        return status;
    }

    sqlite3_bind_text(stmt, 1, data->new_address, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, data->tab_number);

    status.success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!status.success) status.error_message = sqlite3_errmsg(db);

    sqlite3_finalize(stmt);
    return status;
}
