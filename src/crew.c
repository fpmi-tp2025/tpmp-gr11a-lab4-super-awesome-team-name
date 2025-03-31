/* crew.c */
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "crew.h"
#include "validation.h"

// Функция для получения информации о члене экипажа по его табельному номеру
void get_crew_member_info(sqlite3 *db, int tab_number) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT tab_number, last_name, position, experience_years, address, birth_year, helicopter_number "
                        "FROM Crew_member "
                        "WHERE tab_number = ?";
    
    // Подготовка SQL-запроса
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    // Привязка параметров
    sqlite3_bind_int(stmt, 1, tab_number);
    
    // Выполнение запроса и обработка результатов
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("Табельный номер: %d\n", sqlite3_column_int(stmt, 0));
        printf("Фамилия: %s\n", sqlite3_column_text(stmt, 1));
        printf("Должность: %s\n", sqlite3_column_text(stmt, 2));
        printf("Стаж: %d лет\n", sqlite3_column_int(stmt, 3));
        printf("Адрес: %s\n", sqlite3_column_text(stmt, 4));
        printf("Год рождения: %d\n", sqlite3_column_int(stmt, 5));
        printf("Номер вертолета: %d\n", sqlite3_column_int(stmt, 6));
    } else {
        printf("Член экипажа с табельным номером %d не найден.\n", tab_number);
    }
    
    // Освобождение ресурсов
    sqlite3_finalize(stmt);
}

// Функция для получения информации о вертолете, закрепленном за членом экипажа
void get_helicopter_info(sqlite3 *db, int tab_number) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT h.helicopter_number, h.model, h.manufacture_date, h.max_payload, "
                        "h.last_repair_date, h.flight_resource "
                        "FROM Helicopter h "
                        "JOIN Crew_member cm ON h.helicopter_number = cm.helicopter_number "
                        "WHERE cm.tab_number = ?";
    
    // Подготовка SQL-запроса
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    // Привязка параметров
    sqlite3_bind_int(stmt, 1, tab_number);
    
    // Выполнение запроса и обработка результатов
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("Номер вертолета: %d\n", sqlite3_column_int(stmt, 0));
        printf("Модель: %s\n", sqlite3_column_text(stmt, 1));
        printf("Дата изготовления: %s\n", sqlite3_column_text(stmt, 2));
        printf("Максимальная грузоподъемность: %.2f кг\n", sqlite3_column_double(stmt, 3));
        printf("Дата последнего ремонта: %s\n", sqlite3_column_text(stmt, 4));
        printf("Ресурс летного времени: %d часов\n", sqlite3_column_int(stmt, 5));
    } else {
        printf("Не найдена информация о вертолете для члена экипажа с табельным номером %d.\n", tab_number);
    }
    
    // Освобождение ресурсов
    sqlite3_finalize(stmt);
}

// Функция для получения налетанных часов и оставшегося ресурса вертолета члена экипажа
void get_flight_hours_for_crew_helicopter(sqlite3 *db, int tab_number) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT h.helicopter_number, SUM(f.flight_duration), h.flight_resource "
                        "FROM Flight f "
                        "JOIN Helicopter h ON f.helicopter_number = h.helicopter_number "
                        "JOIN Crew_member cm ON h.helicopter_number = cm.helicopter_number "
                        "WHERE cm.tab_number = ? AND f.date > h.last_repair_date "
                        "GROUP BY h.helicopter_number";
    
    // Подготовка SQL-запроса
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    // Привязка параметров
    sqlite3_bind_int(stmt, 1, tab_number);
    
    // Выполнение запроса и обработка результатов
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int helicopter_number = sqlite3_column_int(stmt, 0);
        double total_flight_hours = sqlite3_column_double(stmt, 1);
        int flight_resource = sqlite3_column_int(stmt, 2);
        
        printf("Вертолет %d\n", helicopter_number);
        printf("Общее количество налетанных часов после последнего ремонта: %.2f\n", total_flight_hours);
        printf("Ресурс летного времени: %d часов\n", flight_resource);
        printf("Оставшийся ресурс: %.2f часов\n", flight_resource - total_flight_hours);
    } else {
        printf("Не найдены данные о полетах для члена экипажа с табельным номером %d.\n", tab_number);
    }
    
    // Освобождение ресурсов
    sqlite3_finalize(stmt);
}

// Функция для получения данных о рейсах вертолета члена экипажа за указанный период
void get_flights_by_period_for_crew(sqlite3 *db, int tab_number) {
    char start_date[11], end_date[11];
    
    // Запрос периода
    printf("Введите дату начала периода (ГГГГ-ММ-ДД): ");
    scanf("%10s", start_date);
    printf("Введите дату конца периода (ГГГГ-ММ-ДД): ");
    scanf("%10s", end_date);
    
    // Проверка формата даты
    if (!validate_date(start_date) || !validate_date(end_date)) {
        printf("Неверный формат даты. Используйте формат ГГГГ-ММ-ДД.\n");
        return;
    }
    
    sqlite3_stmt *stmt;
    const char *query = "SELECT f.date, f.flight_code, f.cargo_weight, f.passengers_count, "
                        "f.flight_duration, f.flight_cost, f.is_special "
                        "FROM Flight f "
                        "JOIN Crew_member cm ON f.helicopter_number = cm.helicopter_number "
                        "WHERE cm.tab_number = ? AND f.date BETWEEN ? AND ? "
                        "ORDER BY f.date";
    
    // Подготовка SQL-запроса
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    // Привязка параметров
    sqlite3_bind_int(stmt, 1, tab_number);
    sqlite3_bind_text(stmt, 2, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, end_date, -1, SQLITE_STATIC);
    
    // Переменные для хранения данных
    double total_cargo = 0;
    int total_passengers = 0;
    int flight_count = 0;
    
    // Выполнение запроса и обработка результатов
    printf("\nРейсы с %s по %s:\n", start_date, end_date);
    printf("Дата\t\tКод\tГруз\tПассажиры\tДлительность\tСтоимость\tТип\n");
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *date = (const char*)sqlite3_column_text(stmt, 0);
        int flight_code = sqlite3_column_int(stmt, 1);
        double cargo_weight = sqlite3_column_double(stmt, 2);
        int passengers_count = sqlite3_column_int(stmt, 3);
        double flight_duration = sqlite3_column_double(stmt, 4);
        double flight_cost = sqlite3_column_double(stmt, 5);
        int is_special = sqlite3_column_int(stmt, 6);
        
        printf("%s\t%d\t%.2f\t%d\t\t%.2f\t\t%.2f\t\t%s\n", 
               date, flight_code, cargo_weight, passengers_count, 
               flight_duration, flight_cost, is_special ? "Спецрейс" : "Обычный");
        
        // Обновление итогов
        total_cargo += cargo_weight;
        total_passengers += passengers_count;
        flight_count++;
    }
    
    // Вывод сводки
    if (flight_count > 0) {
        printf("\nСводка:\n");
        printf("Общее количество рейсов: %d\n", flight_count);
        printf("Общая масса груза: %.2f кг\n", total_cargo);
        printf("Общее количество пассажиров: %d\n", total_passengers);
    } else {
        printf("Рейсы в указанном периоде не найдены.\n");
    }
    
    // Освобождение ресурсов
    sqlite3_finalize(stmt);
}

// Функция для расчета денег, заработанных членом экипажа за определенный период
void calculate_crew_member_earnings(sqlite3 *db, int tab_number, const char *start_date, const char *end_date) {
    // Проверка формата даты
    if (!validate_date(start_date) || !validate_date(end_date)) {
        printf("Неверный формат даты. Используйте формат ГГГГ-ММ-ДД.\n");
        return;
    }
    
    sqlite3_stmt *stmt;
    const char *query = "SELECT f.flight_code, f.date, f.flight_cost, f.is_special, f.passengers_count "
                        "FROM Flight f "
                        "JOIN Crew_member cm ON f.helicopter_number = cm.helicopter_number "
                        "WHERE cm.tab_number = ? AND f.date BETWEEN ? AND ? "
                        "ORDER BY f.date";
    
    // Подготовка SQL-запроса
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    // Привязка параметров
    sqlite3_bind_int(stmt, 1, tab_number);
    sqlite3_bind_text(stmt, 2, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, end_date, -1, SQLITE_STATIC);
    
    // Переменные для хранения данных
    double total_earnings = 0;
    int flight_count = 0;
    
    // Выполнение запроса и обработка результатов
    printf("\nЗаработок с %s по %s:\n", start_date, end_date);
    printf("Дата\t\tКод рейса\tСтоимость рейса\tКол-во пассажиров\tТип рейса\tЗаработок\n");
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int flight_code = sqlite3_column_int(stmt, 0);
        const char *date = (const char*)sqlite3_column_text(stmt, 1);
        double flight_cost = sqlite3_column_double(stmt, 2);
        int is_special = sqlite3_column_int(stmt, 3);
        int passengers_count = sqlite3_column_int(stmt,4);
        
        // Расчет заработка (5% для обычных рейсов, 10% для спецрейсов)
        double earnings_percentage = is_special ? 0.10 : 0.05;
        double earnings = (flight_cost * earnings_percentage * passengers_count) / 3;  // Деление на 3 для каждого члена экипажа
        
        printf("%s\t%d\t\t%.2f\t\t%d\t\t\t%s\t\t%.2f\n", 
                date, flight_code, flight_cost, passengers_count , is_special ? "Спецрейс" : "Обычный", earnings);
        
        // Обновление итогов
        total_earnings += earnings;
        flight_count++;
    }
    
    // Вывод сводки
    if (flight_count > 0) {
        printf("\nОбщий заработок за период: %.2f руб.\n", total_earnings);
    } else {
        printf("Рейсы в указанном периоде не найдены.\n");
    }
    
    // Освобождение ресурсов
    sqlite3_finalize(stmt);
}

// Функция для расчета денег, заработанных членом экипажа за конкретный рейс
void calculate_crew_member_earnings_for_flight(sqlite3 *db, int tab_number, int flight_code) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT f.date, f.flight_cost, f.is_special, f.passengers_count "
                        "FROM Flight f "
                        "JOIN Crew_member cm ON f.helicopter_number = cm.helicopter_number "
                        "WHERE cm.tab_number = ? AND f.flight_code = ?";
    
    // Подготовка SQL-запроса
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    // Привязка параметров
    sqlite3_bind_int(stmt, 1, tab_number);
    sqlite3_bind_int(stmt, 2, flight_code);
    
    // Выполнение запроса и обработка результатов
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *date = (const char*)sqlite3_column_text(stmt, 0);
        double flight_cost = sqlite3_column_double(stmt, 1);
        int is_special = sqlite3_column_int(stmt, 2);
        int passengers_count = sqlite3_column_int(stmt, 3);
        
        // Расчет заработка (5% для обычных рейсов, 10% для спецрейсов)
        double earnings_percentage = is_special ? 0.10 : 0.05;
        double earnings = (flight_cost * earnings_percentage * passengers_count) / 3;  // Деление на 3 для каждого члена экипажа
        
        printf("Код рейса: %d\n", flight_code);
        printf("Дата: %s\n", date);
        printf("Стоимость рейса: %.2f руб.\n", flight_cost);
        printf("Количество пассажиров: %d\n", passengers_count);
        printf("Тип рейса: %s\n", is_special ? "Спецрейс" : "Обычный");
        printf("Заработок: %.2f руб.\n", earnings);
    } else {
        printf("Рейс с кодом %d не найден для члена экипажа с табельным номером %d.\n", flight_code, tab_number);
    }
    
    // Освобождение ресурсов
    sqlite3_finalize(stmt);
}

// Функция для получения информации о всех рейсах члена экипажа
void get_all_flights_for_crew(sqlite3 *db, int tab_number) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT f.date, f.flight_code, f.cargo_weight, f.passengers_count, "
                        "f.flight_duration, f.flight_cost, f.is_special "
                        "FROM Flight f "
                        "JOIN Crew_member cm ON f.helicopter_number = cm.helicopter_number "
                        "WHERE cm.tab_number = ? "
                        "ORDER BY f.date";
    
    // Подготовка SQL-запроса
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    // Привязка параметров
    sqlite3_bind_int(stmt, 1, tab_number);
    
    // Выполнение запроса и обработка результатов
    printf("Все рейсы для члена экипажа с табельным номером %d:\n", tab_number);
    printf("Дата\t\tКод\tГруз\tПассажиры\tДлительность\tСтоимость\tТип\n");
    
    int flight_count = 0;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *date = (const char*)sqlite3_column_text(stmt, 0);
        int flight_code = sqlite3_column_int(stmt, 1);
        double cargo_weight = sqlite3_column_double(stmt, 2);
        int passengers_count = sqlite3_column_int(stmt, 3);
        double flight_duration = sqlite3_column_double(stmt, 4);
        double flight_cost = sqlite3_column_double(stmt, 5);
        int is_special = sqlite3_column_int(stmt, 6);
        
        printf("%s\t%d\t%.2f\t%d\t\t%.2f\t\t%.2f\t\t%s\n", 
               date, flight_code, cargo_weight, passengers_count, 
               flight_duration, flight_cost, is_special ? "Спецрейс" : "Обычный");
        
        flight_count++;
    }
    
    if (flight_count == 0) {
        printf("Рейсы для члена экипажа с табельным номером %d не найдены.\n", tab_number);
    }
    
    // Освобождение ресурсов
    sqlite3_finalize(stmt);
}

// Функция для обновления личной информации члена экипажа
int update_crew_member_info(sqlite3 *db, int tab_number) {
    char new_value[100];
    
    // Проверка существования члена экипажа
    sqlite3_stmt *check_stmt;
    const char *check_query = "SELECT COUNT(*) FROM Crew_member WHERE tab_number = ?";
    
    if (sqlite3_prepare_v2(db, check_query, -1, &check_stmt, NULL) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    
    sqlite3_bind_int(check_stmt, 1, tab_number);
    
    if (sqlite3_step(check_stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(check_stmt, 0);
        if (count == 0) {
            printf("Член экипажа с табельным номером %d не существует.\n", tab_number);
            sqlite3_finalize(check_stmt);
            return 1;
        }
    }
    
    sqlite3_finalize(check_stmt);
    
    // Член экипажа может обновить только свой адрес
    printf("Вы можете обновить только свой адрес.\n");
    
    // Получение нового адреса
    printf("Введите новый адрес: ");
    // Очистка буфера ввода
    getchar();
    fgets(new_value, sizeof(new_value), stdin);
    // Удаление символа новой строки
    new_value[strcspn(new_value, "\n")] = 0;
    
    // Обновление адреса в базе данных
    sqlite3_stmt *update_stmt;
    const char *update_query = "UPDATE Crew_member SET address = ? WHERE tab_number = ?";
    
    if (sqlite3_prepare_v2(db, update_query, -1, &update_stmt, NULL) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса обновления: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    
    sqlite3_bind_text(update_stmt, 1, new_value, -1, SQLITE_STATIC);
    sqlite3_bind_int(update_stmt, 2, tab_number);
    
    if (sqlite3_step(update_stmt) != SQLITE_DONE) {
        printf("Ошибка при обновлении адреса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(update_stmt);
        return 1;
    }
    
    printf("Адрес успешно обновлен.\n");
    
    // Освобождение ресурсов
    sqlite3_finalize(update_stmt);
    return 0;
}