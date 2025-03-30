/* commander.c */
#include <stdio.h>
#include <sqlite3.h>
#include <time.h>
#include "commander.h"

// Функция для проверки корректности формата даты (YYYY-MM-DD)
int validate_date(const char *date_str) {
    struct tm tm;
    return strptime(date_str, "%Y-%m-%d", &tm) != NULL;
}

// Функция для преобразования строки в дату (структура tm)
struct tm string_to_date(const char *date_str) {
    struct tm tm = {0};
    strptime(date_str, "%Y-%m-%d", &tm);
    return tm;
}

// Функция для получения и вывода данных по рейсам в указанном периоде
void get_flights_data_by_period(sqlite3 *db) {
    char start_date[11], end_date[11];

    // Запрос периода
    printf("Введите дату начала периода (YYYY-MM-DD): ");
    scanf("%10s", start_date);
    printf("Введите дату конца периода (YYYY-MM-DD): ");
    scanf("%10s", end_date);

    // Валидация формата дат
    if (!validate_date(start_date) || !validate_date(end_date)) {
        printf("Неверный формат даты. Пожалуйста, используйте YYYY-MM-DD.\n");
        return;
    }

    sqlite3_stmt *stmt;
    const char *query = "SELECT h.helicopter_number, f.flight_code, f.cargo_weight, f.passengers_count "
                        "FROM Flight f "
                        "JOIN Helicopter h ON f.helicopter_number = h.helicopter_number "
                        "WHERE f.date BETWEEN ? AND ? "
                        "ORDER BY h.helicopter_number, f.date";

    // Подготовка SQL-запроса
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Привязка параметров
    sqlite3_bind_text(stmt, 1, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, end_date, -1, SQLITE_STATIC);

    // Переменные для хранения данных
    int current_helicopter = -1;
    double total_cargo = 0;
    int total_passengers = 0;

    // Выполнение запроса и обработка результатов
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int helicopter_number = sqlite3_column_int(stmt, 0);
        int flight_code = sqlite3_column_int(stmt, 1);
        double cargo_weight = sqlite3_column_double(stmt, 2);
        int passengers_count = sqlite3_column_int(stmt, 3);

        if (helicopter_number != current_helicopter) {
            if (current_helicopter != -1) {
                // Выводим данные для предыдущего вертолета
                printf("Общая масса: %.2f, количество человек: %d\n", total_cargo, total_passengers);
            }

            // Новый вертолет
            printf("\nHelicopter %d\n", helicopter_number);
            current_helicopter = helicopter_number;
            total_cargo = 0;
            total_passengers = 0;
        }

        // Вывод данных о рейсе
        printf("%d %.2f %d\n", flight_code, cargo_weight, passengers_count);

        // Обновление общей массы и количества пассажиров
        total_cargo += cargo_weight;
        total_passengers += passengers_count;
    }

    // Вывод итогов для последнего вертолета
    if (current_helicopter != -1) {
        printf("Общая масса: %.2f, количество человек: %d\n", total_cargo, total_passengers);
    }

    // Освобождение ресурсов
    sqlite3_finalize(stmt);
}

// Функция для получения налетанных часов и ресурса летного времени после капитального ремонта
void get_flights_hours_after_repair(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT h.helicopter_number, SUM(f.flight_duration), h.flight_resource "
                        "FROM Flight f "
                        "JOIN Helicopter h ON f.helicopter_number = h.helicopter_number "
                        "WHERE f.date > h.last_repair_date "
                        "GROUP BY h.helicopter_number";

    // Подготовка SQL-запроса
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Выполнение запроса и обработка результатов
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int helicopter_number = sqlite3_column_int(stmt, 0);
        double total_flight_hours = sqlite3_column_double(stmt, 1);
        int flight_resource = sqlite3_column_int(stmt, 2);

        // Выводим информацию для каждого вертолета
        printf("Helicopter %d\n", helicopter_number);
        printf("Налетанные часы после капитального ремонта: %.2f\n", total_flight_hours);
        printf("Ресурс летного времени: %d\n", flight_resource);
        printf("\n");
    }

    // Освобождение ресурсов
    sqlite3_finalize(stmt);
}

// Функция для получения общего количества рейсов, массы грузов и суммы заработанных денег по спецрейсам
void get_special_flights_summary(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT h.helicopter_number, COUNT(f.flight_code), SUM(f.cargo_weight), SUM(f.flight_cost) "
                        "FROM Flight f "
                        "JOIN Helicopter h ON f.helicopter_number = h.helicopter_number "
                        "WHERE f.is_special = 1 "
                        "GROUP BY h.helicopter_number";

    // Подготовка SQL-запроса
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Выполнение запроса и обработка результатов
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int helicopter_number = sqlite3_column_int(stmt, 0);
        int flight_count = sqlite3_column_int(stmt, 1);
        double total_cargo_weight = sqlite3_column_double(stmt, 2);
        double total_income = sqlite3_column_double(stmt, 3);

        // Выводим информацию для каждого вертолета
        printf("Helicopter %d\n", helicopter_number);
        printf("Количество спецрейсов: %d\n", flight_count);
        printf("Общая масса перевезенных грузов: %.2f\n", total_cargo_weight);
        printf("Общая сумма заработанных денег: %.2f$\n", total_income);
        printf("\n");
    }

    // Освобождение ресурсов
    sqlite3_finalize(stmt);
}

// Функция для вывода максимально заработавшего экипажа
void get_max_earning_crew(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql_crew =
            "SELECT H.helicopter_number, H.model, SUM(F.flight_cost) AS total_earnings "
            "FROM Flight F "
            "JOIN Crew_member CM ON CM.helicopter_number = F.helicopter_number "
            "JOIN Helicopter H ON F.helicopter_number = H.helicopter_number "
            "GROUP BY F.helicopter_number "
            "ORDER BY total_earnings DESC LIMIT 1";  // Получаем вертолет с максимальными заработками

    int rc = sqlite3_prepare_v2(db, sql_crew, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Получаем информацию о вертолете и заработанных деньгах
        int helicopter_number = sqlite3_column_int(stmt, 0);
        const char *helicopter_model = (const char*)sqlite3_column_text(stmt, 1);
        double total_earnings = sqlite3_column_double(stmt, 2);

        // Выводим информацию о вертолете и заработанных деньгах
        printf("Номер вертолета: %d\n", helicopter_number);
        printf("Модель вертолета: %s\n", helicopter_model);
        printf("Общее количество заработанных денег: %.2f$\n", total_earnings);

        // Теперь выводим членов экипажа, связанного с этим вертолетом
        const char *sql_crew_members =
                "SELECT CM.tab_number, CM.last_name "
                "FROM Crew_member CM "
                "WHERE CM.helicopter_number = ?";  // Получаем членов экипажа для данного вертолета

        sqlite3_stmt *stmt_members;
        rc = sqlite3_prepare_v2(db, sql_crew_members, -1, &stmt_members, 0);
        if (rc != SQLITE_OK) {
            printf("Ошибка при подготовке запроса для членов экипажа: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return;
        }

        sqlite3_bind_int(stmt_members, 1, helicopter_number);

        printf("Члены экипажа:\n");

        while (sqlite3_step(stmt_members) == SQLITE_ROW) {
            int tab_number = sqlite3_column_int(stmt_members, 0);
            const char *last_name = (const char*)sqlite3_column_text(stmt_members, 1);
            printf("Табельный номер: %d, Фамилия: %s\n", tab_number, last_name);
        }

        sqlite3_finalize(stmt_members);

        // Далее выводим рейсы этого экипажа
        const char *sql_flights =
                "SELECT F.date, F.flight_code, F.flight_cost "
                "FROM Flight F "
                "WHERE F.helicopter_number = ?";  // Получаем все рейсы для этого вертолета

        sqlite3_stmt *stmt_flights;
        rc = sqlite3_prepare_v2(db, sql_flights, -1, &stmt_flights, 0);
        if (rc != SQLITE_OK) {
            printf("Ошибка при подготовке запроса для рейсов: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return;
        }

        sqlite3_bind_int(stmt_flights, 1, helicopter_number);

        printf("Рейсы:\n");

        while (sqlite3_step(stmt_flights) == SQLITE_ROW) {
            const char *flight_date = (const char*)sqlite3_column_text(stmt_flights, 0);
            int flight_code = sqlite3_column_int(stmt_flights, 1);
            double flight_cost = sqlite3_column_double(stmt_flights, 2);
            printf("Дата: %s, Код полета: %d, Стоимость рейса: %.2f$\n", flight_date, flight_code, flight_cost);
        }

        sqlite3_finalize(stmt_flights);
    } else {
        printf("Не удалось найти экипаж с максимальными заработками.\n");
    }

    sqlite3_finalize(stmt);
}
