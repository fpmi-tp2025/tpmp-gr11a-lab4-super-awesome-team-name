/* commander.c */
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/commander/commander.h"

// Функция для получения и данных по рейсам в указанном периоде
FlightPeriodReport* get_flights_report(sqlite3 *db, const char *start, const char *end) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT h.helicopter_number, f.flight_code, f.cargo_weight, f.passengers_count "
                      "FROM Flight f "
                      "JOIN Helicopter h ON f.helicopter_number = h.helicopter_number "
                      "WHERE f.date BETWEEN ? AND ? "
                      "ORDER BY h.helicopter_number, f.date;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, start, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, end, -1, SQLITE_STATIC);

    FlightPeriodReport *root = NULL, *current = NULL;
    int prev_helicopter = -1;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int heli = sqlite3_column_int(stmt, 0);
        int code = sqlite3_column_int(stmt, 1);
        double cargo = sqlite3_column_double(stmt, 2);
        int pass = sqlite3_column_int(stmt, 3);

        if (heli != prev_helicopter) {
            FlightPeriodReport *new_node = calloc(1, sizeof(FlightPeriodReport));
            new_node->helicopter_number = heli;
            new_node->next = NULL;

            if (!root) root = new_node;
            else current->next = new_node;

            current = new_node;
            prev_helicopter = heli;
        }

        current->total_flights++;
        current->flight_codes = realloc(current->flight_codes, current->total_flights * sizeof(int));
        current->cargo_weights = realloc(current->cargo_weights, current->total_flights * sizeof(double));
        current->passengers = realloc(current->passengers, current->total_flights * sizeof(int));

        current->flight_codes[current->total_flights-1] = code;
        current->cargo_weights[current->total_flights-1] = cargo;
        current->passengers[current->total_flights-1] = pass;

        current->total_cargo += cargo;
        current->total_passengers += pass;
    }

    sqlite3_finalize(stmt);
    return root;
}


// Функция для получения налетанных часов и ресурса летного времени после капитального ремонта
HelicopterHours* retrieve_flight_hours_data(sqlite3 *db, int *result_count) {
    sqlite3_stmt *stmt;
    HelicopterHours *results = NULL;
    int count = 0;
    const char *query = "SELECT h.helicopter_number, SUM(f.flight_duration), h.flight_resource "
                        "FROM Flight f "
                        "JOIN Helicopter h ON f.helicopter_number = h.helicopter_number "
                        "WHERE f.date > h.last_repair_date "
                        "GROUP BY h.helicopter_number";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    // Первый проход для подсчета записей
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }
    sqlite3_reset(stmt);

    if (count == 0) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    results = malloc(sizeof(HelicopterHours) * count);
    if (!results) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    // Заполнение данных
    for (int i = 0; i < count; i++) {
        sqlite3_step(stmt);
        results[i].helicopter_number = sqlite3_column_int(stmt, 0);
        results[i].total_flight_hours = sqlite3_column_double(stmt, 1);
        results[i].flight_resource = sqlite3_column_int(stmt, 2);
    }

    *result_count = count;
    sqlite3_finalize(stmt);
    return results;
}

// Функция для получения общего количества рейсов, массы грузов и суммы заработанных денег по спецрейсам
SpecialFlightsSummary* retrieve_special_flights_data(sqlite3 *db, int *result_count) {
    sqlite3_stmt *stmt;
    SpecialFlightsSummary *results = NULL;
    int count = 0;
    const char *query = "SELECT h.helicopter_number, COUNT(f.flight_code), SUM(f.cargo_weight), SUM(f.flight_cost) "
                        "FROM Flight f "
                        "JOIN Helicopter h ON f.helicopter_number = h.helicopter_number "
                        "WHERE f.is_special = 1 "
                        "GROUP BY h.helicopter_number";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    // Подсчет количества записей
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }
    sqlite3_reset(stmt);

    if (count == 0) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    // Выделение памяти
    results = malloc(sizeof(SpecialFlightsSummary) * count);
    if (!results) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    // Заполнение структур
    for (int i = 0; i < count; i++) {
        sqlite3_step(stmt);
        results[i].helicopter_number = sqlite3_column_int(stmt, 0);
        results[i].flights_count = sqlite3_column_int(stmt, 1);
        results[i].total_cargo_weight = sqlite3_column_double(stmt, 2);
        results[i].total_income = sqlite3_column_double(stmt, 3);
    }

    *result_count = count;
    sqlite3_finalize(stmt);
    return results;
}

max_earning_crew_t retrieve_max_earning_crew_data(sqlite3 *db) {
    sqlite3_stmt *stmt;
    max_earning_crew_t result = {0};
    const char *main_query =
            "SELECT H.helicopter_number, H.model, SUM(F.flight_cost) "
            "FROM Flight F "
            "JOIN Helicopter H ON F.helicopter_number = H.helicopter_number "
            "GROUP BY F.helicopter_number "
            "ORDER BY SUM(F.flight_cost) DESC LIMIT 1";

    // Основной запрос
    if (sqlite3_prepare_v2(db, main_query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки основного запроса: %s\n", sqlite3_errmsg(db));
        return result;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Заполнение основных полей
        result.helicopter_number = sqlite3_column_int(stmt, 0);
        const unsigned char *model = sqlite3_column_text(stmt, 1);
        strncpy(result.helicopter_model, (const char *) model, sizeof(result.helicopter_model) - 1);
        result.total_earnings = sqlite3_column_double(stmt, 2);

        // Запрос членов экипажа
        const char *crew_query =
                "SELECT tab_number, last_name FROM Crew_member "
                "WHERE helicopter_number = ?";
        sqlite3_stmt *crew_stmt;

        if (sqlite3_prepare_v2(db, crew_query, -1, &crew_stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(crew_stmt, 1, result.helicopter_number);

            while (sqlite3_step(crew_stmt) == SQLITE_ROW && result.crew_count < 10) {
                result.crew_members[result.crew_count].tab_number = sqlite3_column_int(crew_stmt, 0);
                const unsigned char *name = sqlite3_column_text(crew_stmt, 1);
                strncpy(result.crew_members[result.crew_count].last_name,
                        (const char *) name,
                        sizeof(result.crew_members[0].last_name) - 1);
                result.crew_count++;
            }
            sqlite3_finalize(crew_stmt);
        }

        // Запрос рейсов
        const char *flights_query =
                "SELECT date, flight_code, flight_cost FROM Flight "
                "WHERE helicopter_number = ?";
        sqlite3_stmt *flights_stmt;

        if (sqlite3_prepare_v2(db, flights_query, -1, &flights_stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(flights_stmt, 1, result.helicopter_number);

            while (sqlite3_step(flights_stmt) == SQLITE_ROW && result.flight_count < 10) {
                const unsigned char *date = sqlite3_column_text(flights_stmt, 0);
                strncpy(result.flights[result.flight_count].date,
                        (const char *) date,
                        sizeof(result.flights[0].date) - 1);
                result.flights[result.flight_count].flight_code = sqlite3_column_int(flights_stmt, 1);
                result.flights[result.flight_count].flight_cost = sqlite3_column_double(flights_stmt, 2);
                result.flight_count++;
            }
            sqlite3_finalize(flights_stmt);
        }
    }

    sqlite3_finalize(stmt);
    return result;
}

// Информацию по вертолету и экипажу с макс кол-во рейсов
HelicopterWithCrewData* get_helicopter_with_crew_most_flights(sqlite3 *db) {
    sqlite3_stmt *stmt;
    HelicopterWithCrewData *result = NULL;
    
    // SQL запрос для нахождения вертолета, выполнившего наибольшее количество рейсов
    const char *sql_heli =
        "SELECT H.helicopter_number, H.model, COUNT(F.flight_code) AS num_flights, SUM(F.flight_cost) AS total_earnings "
        "FROM Flight F "
        "JOIN Helicopter H ON F.helicopter_number = H.helicopter_number "
        "GROUP BY F.helicopter_number "
        "ORDER BY num_flights DESC LIMIT 1";
    
    int rc = sqlite3_prepare_v2(db, sql_heli, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        return NULL;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Выделяем память для структуры результата
        result = (HelicopterWithCrewData*)malloc(sizeof(HelicopterWithCrewData));
        if (!result) {
            sqlite3_finalize(stmt);
            return NULL;
        }
        
        // Заполняем данные о вертолете
        result->helicopter.helicopter_number = sqlite3_column_int(stmt, 0);
        result->helicopter.helicopter_model = strdup((const char*)sqlite3_column_text(stmt, 1));
        result->helicopter.num_flights = sqlite3_column_int(stmt, 2);
        result->helicopter.total_earnings = sqlite3_column_double(stmt, 3);
        
        // Получаем информацию о членах экипажа для этого вертолета
        sqlite3_stmt *stmt_crew;
        const char *sql_crew =
            "SELECT CM.tab_number, CM.last_name "
            "FROM Crew_member CM "
            "WHERE CM.helicopter_number = ?";
        
        rc = sqlite3_prepare_v2(db, sql_crew, -1, &stmt_crew, 0);
        if (rc != SQLITE_OK) {
            free((void*)result->helicopter.helicopter_model);
            free(result);
            sqlite3_finalize(stmt);
            return NULL;
        }
        
        sqlite3_bind_int(stmt_crew, 1, result->helicopter.helicopter_number);
        
        // Сначала подсчитываем количество членов экипажа
        int crew_count = 0;
        while (sqlite3_step(stmt_crew) == SQLITE_ROW) {
            crew_count++;
        }
        
        // Сбрасываем запрос для повторного выполнения
        sqlite3_reset(stmt_crew);
        sqlite3_bind_int(stmt_crew, 1, result->helicopter.helicopter_number);
        
        // Выделяем память для массива членов экипажа
        result->crew_members = (CrewMemberData*)malloc(crew_count * sizeof(CrewMemberData));
        result->crew_count = crew_count;
        
        if (!result->crew_members) {
            free((void*)result->helicopter.helicopter_model);
            free(result);
            sqlite3_finalize(stmt_crew);
            sqlite3_finalize(stmt);
            return NULL;
        }
        
        // Заполняем данные о членах экипажа
        int i = 0;
        while (sqlite3_step(stmt_crew) == SQLITE_ROW && i < crew_count) {
            result->crew_members[i].tab_number = sqlite3_column_int(stmt_crew, 0);
            result->crew_members[i].last_name = strdup((const char*)sqlite3_column_text(stmt_crew, 1));
            i++;
        }
        
        sqlite3_finalize(stmt_crew);
    }
    
    sqlite3_finalize(stmt);
    return result;
}

// Данные по вертолетам проводившие обычный рейс
HelicopterSummary* retrieve_normal_flights_data(sqlite3 *db, int* result_count) {
    sqlite3_stmt *stmt;
    HelicopterSummary* results = NULL;
    *result_count = 0;

    const char *sql =
            "SELECT H.helicopter_number, H.model, COUNT(F.flight_code), "
            "SUM(F.cargo_weight), SUM(F.flight_cost) "
            "FROM Flight F "
            "JOIN Helicopter H ON F.helicopter_number = H.helicopter_number "
            "WHERE F.is_special = 0 "
            "GROUP BY F.helicopter_number";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    // Первый проход для подсчета записей
    while (sqlite3_step(stmt) == SQLITE_ROW) (*result_count)++;
    sqlite3_reset(stmt);

    if (*result_count == 0) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    results = malloc(sizeof(HelicopterSummary) * (*result_count));
    if (!results) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    // Второй проход для заполнения данных
    for (int i = 0; i < *result_count; i++) {
        sqlite3_step(stmt);

        results[i].helicopter_number = sqlite3_column_int(stmt, 0);

        const char* model = (const char*)sqlite3_column_text(stmt, 1);
        results[i].model = model ? strdup(model) : strdup("N/A");

        results[i].num_flights = sqlite3_column_int(stmt, 2);
        results[i].total_cargo_weight = sqlite3_column_double(stmt, 3);
        results[i].total_earnings = sqlite3_column_double(stmt, 4);
    }

    sqlite3_finalize(stmt);
    return results;
}

// Обновление члена экипажа
int update_crew_member_db(sqlite3 *db, int tab_number, const char *field, const char *new_value) {
    // Проверка допустимых полей
    const char *allowed_fields[] = {"last_name", "position", "birth_year", "address", "helicopter_number"};
    int valid_field = 0;
    for (size_t i = 0; i < sizeof(allowed_fields)/sizeof(allowed_fields[0]); i++) {
        if (strcmp(field, allowed_fields[i]) == 0) {
            valid_field = 1;
            break;
        }
    }
    if (!valid_field) return 0;

    // Валидация данных
    if (strcmp(field, "last_name") == 0 && !validate_name(new_value)) return 0;
    if (strcmp(field, "position") == 0 && !validate_position(new_value)) return 0;
    if (strcmp(field, "birth_year") == 0 && !validate_birth_year(new_value)) return 0;
    if (strcmp(field, "helicopter_number") == 0 && !validate_helicopter_number(db, atoi(new_value))) return 0;

    // Формирование параметризованного запроса
    const char *sql = "UPDATE Crew_member SET ? = ? WHERE tab_number = ?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;

    // Привязка параметров
    sqlite3_bind_text(stmt, 1, field, -1, SQLITE_STATIC);

    if (strcmp(field, "helicopter_number") == 0) {
        sqlite3_bind_int(stmt, 2, atoi(new_value));
    } else {
        sqlite3_bind_text(stmt, 2, new_value, -1, SQLITE_STATIC);
    }

    sqlite3_bind_int(stmt, 3, tab_number);

    // Выполнение запроса
    int result = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    return result;
}

// Обновить данные о рейсе
int update_flight(sqlite3 *db) {
    int flight_code;
    char field[50];
    char new_value[100];
    char sql[256];

    // Запрос flight_code для выбора рейса
    while (1) {
        printf("Введите код полета (flight_code): ");
        scanf("%d", &flight_code);

        // Проверка, существует ли такой рейс
        if (!validate_flight_code(db, flight_code)) {
            printf("Ошибка: рейс с таким кодом не найден. Попробуйте снова.\n");
        } else {
            break;  // Если рейс найден, выходим из цикла
        }
    }

    // Запрос выбора поля для обновления
    while (1) {
        printf("Выберите поле для обновления:\n");
        printf("1. cargo_weight\n");
        printf("2. helicopter_number\n");
        printf("3. passengers_count\n");
        printf("4. flight_duration\n");
        printf("5. flight_cost\n");
        printf("6. is_special (0, 1, нет, да)\n");
        printf("Введите название поля: ");
        scanf("%s", field);

        // Проверка, существует ли выбранное поле
        if (
                strcmp(field, "cargo_weight") == 0 ||
                strcmp(field, "helicopter_number") == 0 ||
                strcmp(field, "passengers_count") == 0 ||
                strcmp(field, "flight_duration") == 0 ||
                strcmp(field, "flight_cost") == 0 ||
                strcmp(field, "is_special") == 0
                ) {
            break;  // Если поле валидное, выходим из цикла
        } else {
            printf("Ошибка: неверное поле. Попробуйте снова.\n");
        }
    }

    // Запрос нового значения для этого поля
    printf("Введите новое значение для поля %s: ", field);
    scanf("%s", new_value);

    // === ВАЛИДАЦИЯ ПО ПОЛЮ ===

    if (strcmp(field, "cargo_weight") == 0) {
        if (!validate_float(new_value)) {
            printf("Ошибка: некорректный формат числа с плавающей точкой.\n");
            return 1;
        }
        float weight = atof(new_value);

        // Получаем максимальную массу для вертолета рейса
        sqlite3_stmt *stmt;
        const char *sql_get_heli = "SELECT helicopter_number FROM Flight WHERE flight_code = ?";
        if (sqlite3_prepare_v2(db, sql_get_heli, -1, &stmt, 0) != SQLITE_OK) {
            printf("Ошибка запроса: %s\n", sqlite3_errmsg(db));
            return 1;
        }

        sqlite3_bind_int(stmt, 1, flight_code);
        int heli_num = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            heli_num = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);

        if (heli_num == -1) {
            printf("Ошибка: рейс не найден.\n");
            return 1;
        }

        // Получаем max_payload
        const char *sql_payload = "SELECT max_payload FROM Helicopter WHERE helicopter_number = ?";
        if (sqlite3_prepare_v2(db, sql_payload, -1, &stmt, 0) != SQLITE_OK) {
            printf("Ошибка запроса: %s\n", sqlite3_errmsg(db));
            return 1;
        }

        sqlite3_bind_int(stmt, 1, heli_num);
        float max_payload = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            max_payload = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);

        if (weight > max_payload) {
            printf("Ошибка: груз превышает максимальную грузоподъемность вертолета (%.2f).\n", max_payload);
            return 1;
        }

    } else if (strcmp(field, "helicopter_number") == 0) {
        int h = atoi(new_value);
        if (!validate_helicopter_number(db, h)) {
            printf("Ошибка: вертолет с номером %d не найден.\n", h);
            return 1;
        }

    } else if (
            strcmp(field, "passengers_count") == 0 ||
            strcmp(field, "flight_duration") == 0 ||
            strcmp(field, "flight_cost") == 0
            ) {
        if (!validate_float(new_value)) {
            printf("Ошибка: введите корректное число.\n");
            return 1;
        }

    } else if (strcmp(field, "is_special") == 0) {
        int val = validate_is_special(new_value);
        if (val == -1) {
            printf("Ошибка: значение может быть только 0, 1, нет, да.\n");
            return 1;
        }
        // Перезаписываем new_value числовым значением
        snprintf(new_value, sizeof(new_value), "%d", val);

    } else {
        printf("Ошибка: поле \"%s\" не поддерживается для обновления.\n", field);
        return 1;
    }

    // === ОБНОВЛЕНИЕ В БАЗЕ ===

    snprintf(sql, sizeof(sql), "UPDATE Flight SET %s = ? WHERE flight_code = ?", field);
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Привязка значения
    if (strcmp(field, "cargo_weight") == 0 || strcmp(field, "flight_cost") == 0) {
        sqlite3_bind_double(stmt, 1, atof(new_value));
    } else if (strcmp(field, "is_special") == 0 ||
               strcmp(field, "passengers_count") == 0 ||
               strcmp(field, "flight_duration") == 0 ||
               strcmp(field, "helicopter_number") == 0) {
        sqlite3_bind_int(stmt, 1, atoi(new_value));
    }

    sqlite3_bind_int(stmt, 2, flight_code);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Ошибка при обновлении: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }

    printf("Поле \"%s\" успешно обновлено.\n", field);
    sqlite3_finalize(stmt);
    return 0;
}

// Обновить данные по вертолетам
int update_helicopter(sqlite3 *db) {
    int helicopter_number;
    char field[50];
    char new_value[100];
    char sql[256];

    // Запрос helicopter_number
    printf("Введите номер вертолета для обновления: ");
    scanf("%d", &helicopter_number);

    // Проверка существования вертолета
    if (!validate_helicopter_number(db, helicopter_number)) {
        printf("Ошибка: вертолет с номером %d не найден.\n", helicopter_number);
        return 1;
    }

    // Запрос выбора поля для обновления
    printf("Выберите поле для обновления:\n");
    printf("1. model\n");
    printf("2. manufacture_date\n");
    printf("3. max_payload\n");
    printf("4. last_repair_date\n");
    printf("5. flight_resource\n");
    printf("Введите название поля: ");
    scanf("%s", field);

    // Запрос нового значения для этого поля
    printf("Введите новое значение для поля %s: ", field);
    scanf("%s", new_value);

    // === ВАЛИДАЦИЯ ПО ПОЛЮ ===

    if (strcmp(field, "model") == 0) {
        // Проверка модели (строка)
        if (!validate_name(new_value)) {
            printf("Ошибка: некорректная модель. Используйте только буквы.\n");
            return 1;
        }
    } else if (strcmp(field, "manufacture_date") == 0 || strcmp(field, "last_repair_date") == 0) {
        // Проверка даты
        if (!validate_date(new_value)) {
            printf("Ошибка: некорректная дата. Используйте формат YYYY-MM-DD.\n");
            return 1;
        }
    } else if (strcmp(field, "max_payload") == 0 || strcmp(field, "flight_resource") == 0) {
        // Проверка числовых значений
        if (!validate_float(new_value)) {
            printf("Ошибка: некорректное число. Введите корректное числовое значение.\n");
            return 1;
        }
    } else {
        printf("Ошибка: поле \"%s\" не поддерживается для обновления.\n", field);
        return 1;
    }

    // === ОБНОВЛЕНИЕ В БАЗЕ ===

    snprintf(sql, sizeof(sql), "UPDATE Helicopter SET %s = ? WHERE helicopter_number = ?", field);
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Привязка значений
    if (strcmp(field, "model") == 0) {
        sqlite3_bind_text(stmt, 1, new_value, -1, SQLITE_STATIC);
    } else if (strcmp(field, "manufacture_date") == 0 || strcmp(field, "last_repair_date") == 0) {
        sqlite3_bind_text(stmt, 1, new_value, -1, SQLITE_STATIC);
    } else if (strcmp(field, "max_payload") == 0 || strcmp(field, "flight_resource") == 0) {
        sqlite3_bind_double(stmt, 1, atof(new_value));
    }

    sqlite3_bind_int(stmt, 2, helicopter_number);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Ошибка при обновлении: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }

    printf("Данные успешно обновлены.\n");

    sqlite3_finalize(stmt);
    return 0;
}

// Вставка работника в БД
int insert_crew_member(sqlite3 *db) {
    int tab_number, birth_year, experience_year, helicopter_number;
    char last_name[100], position[50], address[200];
    char input[100];

    // === tab_number ===
    while (1) {
        printf("Введите табельный номер (целое положительное число): ");
        scanf("%s", input);
        if (validate_number(input)) {
            tab_number = atoi(input);
            break;
        } else {
            printf("Ошибка: некорректный табельный номер.\n");
        }
    }

    // === last_name ===
    while (1) {
        printf("Введите фамилию (только буквы): ");
        scanf("%s", last_name);
        if (validate_name(last_name)) {
            break;
        } else {
            printf("Ошибка: фамилия должна содержать только буквы.\n");
        }
    }

    // === position ===
    while (1) {
        printf("Введите должность (commander или crew_member): ");
        scanf("%s", position);
        if (validate_position(position)) {
            break;
        } else {
            printf("Ошибка: допустимые значения – commander или crew_member.\n");
        }
    }

    // === birth_year ===
    while (1) {
        printf("Введите год рождения (4 цифры): ");
        scanf("%s", input);
        if (validate_birth_year(input)) {
            birth_year = atoi(input);
            break;
        } else {
            printf("Ошибка: введите корректный 4-значный год.\n");
        }
    }

    // === experience_year ===
    while (1) {
        printf("Введите количество лет опыта (целое число >= 0): ");
        scanf("%s", input);
        if (validate_number(input)) {
            experience_year = atoi(input);
            break;
        } else {
            printf("Ошибка: опыт должен быть целым числом (>= 0).\n");
        }
    }

    // === address ===
    printf("Введите адрес: ");
    getchar(); // очистка \n после предыдущего scanf
    fgets(address, sizeof(address), stdin);
    address[strcspn(address, "\n")] = 0;

    // === helicopter_number ===
    while (1) {
        printf("Введите номер вертолета: ");
        scanf("%s", input);
        helicopter_number = atoi(input);
        if (validate_number(input) && validate_helicopter_number(db, helicopter_number)) {
            break;
        } else {
            printf("Ошибка: вертолет с таким номером не найден или номер некорректен.\n");
        }
    }

    // === Вставка в базу ===
    const char *sql = "INSERT INTO Crew_member (tab_number, last_name, position, birth_year, experience_years, address, helicopter_number) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?)";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_int(stmt, 1, tab_number);
    sqlite3_bind_text(stmt, 2, last_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, position, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, birth_year);
    sqlite3_bind_int(stmt, 5, experience_year);
    sqlite3_bind_text(stmt, 6, address, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, helicopter_number);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Ошибка при вставке: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    printf("Член экипажа успешно добавлен.\n");
    return 0;
}

int insert_helicopter(sqlite3 *db) {
    int helicopter_number, flight_resource;
    char model[100], manufacture_date[20], last_repair_date[20];
    char input[100];
    float max_payload;
    
    // === helicopter_number ===
    while (1) {
        printf("Введите номер вертолёта (целое положительное число): ");
        scanf("%s", input);
        if (validate_number(input)) {
            helicopter_number = atoi(input);
            // Проверка, что такого номера ещё нет в базе
            sqlite3_stmt *stmt;
            const char *check_sql = "SELECT COUNT(*) FROM Helicopter WHERE helicopter_number = ?";
            if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, 0) != SQLITE_OK) {
                printf("Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
                return 1;
            }
            sqlite3_bind_int(stmt, 1, helicopter_number);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int count = sqlite3_column_int(stmt, 0);
                sqlite3_finalize(stmt);
                if (count > 0) {
                    printf("Ошибка: вертолёт с таким номером уже существует.\n");
                    continue;
                }
            } else {
                sqlite3_finalize(stmt);
                printf("Ошибка при проверке номера вертолёта.\n");
                continue;
            }
            break;
        } else {
            printf("Ошибка: некорректный номер вертолёта.\n");
        }
    }
    
    // === model ===
    printf("Введите модель вертолёта: ");
    getchar(); // очистка буфера ввода
    fgets(model, sizeof(model), stdin);
    model[strcspn(model, "\n")] = 0; // удаляем символ новой строки
    
    // === manufacture_date ===
    while (1) {
        printf("Введите дату производства (YYYY-MM-DD): ");
        scanf("%s", manufacture_date);
        if (validate_date(manufacture_date)) {
            break;
        } else {
            printf("Ошибка: некорректный формат даты. Используйте формат YYYY-MM-DD.\n");
        }
    }
    
    // === max_payload ===
    while (1) {
        printf("Введите максимальную грузоподъёмность (в кг): ");
        scanf("%s", input);
        if (validate_float(input)) {
            max_payload = atof(input);
            break;
        } else {
            printf("Ошибка: грузоподъёмность должна быть положительным числом.\n");
        }
    }
    
    // === last_repair_date ===
    while (1) {
        printf("Введите дату последнего ремонта (YYYY-MM-DD): ");
        scanf("%s", last_repair_date);
        if (validate_date(last_repair_date)) {
            break;
        } else {
            printf("Ошибка: некорректный формат даты. Используйте формат YYYY-MM-DD.\n");
        }
    }
    
    // === flight_resource ===
    while (1) {
        printf("Введите ресурс полёта (целое положительное число): ");
        scanf("%s", input);
        if (validate_number(input)) {
            flight_resource = atoi(input);
            break;
        } else {
            printf("Ошибка: ресурс полёта должен быть целым положительным числом.\n");
        }
    }
    
    // === Вставка в базу ===
    const char *sql = "INSERT INTO Helicopter (helicopter_number, model, manufacture_date, max_payload, last_repair_date, flight_resource) "
                    "VALUES (?, ?, ?, ?, ?, ?)";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    
    sqlite3_bind_int(stmt, 1, helicopter_number);
    sqlite3_bind_text(stmt, 2, model, -1, SQLITE_TRANSIENT);  // Изменено с SQLITE_STATIC на SQLITE_TRANSIENT
    sqlite3_bind_text(stmt, 3, manufacture_date, -1, SQLITE_TRANSIENT);  // Изменено с SQLITE_STATIC на SQLITE_TRANSIENT
    sqlite3_bind_double(stmt, 4, max_payload);
    sqlite3_bind_text(stmt, 5, last_repair_date, -1, SQLITE_TRANSIENT);  // Изменено с SQLITE_STATIC на SQLITE_TRANSIENT
    sqlite3_bind_int(stmt, 6, flight_resource);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Ошибка при вставке: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }
    
    sqlite3_finalize(stmt);
    printf("Вертолёт успешно добавлен.\n");
    return 0;
}

// Удаление члена экипажа
int delete_crew_member(sqlite3 *db) {
    int tab_number;

    // Запрос табельного номера
    printf("Введите табельный номер члена экипажа для удаления: ");
    scanf("%d", &tab_number);

    // Проверка, существует ли такой член экипажа в базе данных
    if (!validate_crew_member(db, tab_number)) {
        printf("Ошибка: Член экипажа с табельным номером %d не найден.\n", tab_number);
        return 1;  // Завершаем функцию, если член экипажа не найден
    }

    // Формирование SQL запроса для удаления члена экипажа
    const char *sql = "DELETE FROM Crew_member WHERE tab_number = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Привязка табельного номера
    sqlite3_bind_int(stmt, 1, tab_number);

    // Выполнение запроса на удаление
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        printf("Ошибка при удалении члена экипажа: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }

    printf("Член экипажа с табельным номером %d успешно удален.\n", tab_number);

    // Завершаем работу с запросом
    sqlite3_finalize(stmt);
    return 0;
}

// Вспомогательная функция для получения ответа Да/Нет
int get_yes_no() {
    char response[10];
    printf("(y/n): ");
    scanf("%s", response);
    
    // Проверяем первый символ ответа
    char first_char = response[0];
    return (first_char == 'y' || first_char == 'Y' || first_char == '1');
}

// Удаление вертолёта
int delete_helicopter(sqlite3 *db) {
    int helicopter_number;
    
    // Запрос номера вертолёта
    printf("Введите номер вертолёта для удаления: ");
    scanf("%d", &helicopter_number);
    
    // Проверка, существует ли такой вертолёт в базе данных
    if (!validate_helicopter(db, helicopter_number)) {
        printf("Ошибка: Вертолёт с номером %d не найден.\n", helicopter_number);
        return 1; // Завершаем функцию, если вертолёт не найден
    }
    
    // Проверка, привязаны ли члены экипажа к этому вертолёту
    int crew_count = check_crew_members_for_helicopter(db, helicopter_number);
    if (crew_count > 0) {
        printf("Внимание: К вертолёту с номером %d привязаны %d членов экипажа.\n", 
               helicopter_number, crew_count);
        printf("Удаление вертолёта повлечёт за собой удаление связанных записей!\n");
        printf("Хотите продолжить? ");
        
        if (!get_yes_no()) {
            printf("Операция отменена.\n");
            return 0;
        }
    }
    
    // Проверка, есть ли полёты, связанные с этим вертолётом
    int flight_count = check_flights_for_helicopter(db, helicopter_number);
    if (flight_count > 0) {
        printf("Внимание: С вертолётом номер %d связаны %d полётов.\n", 
               helicopter_number, flight_count);
        printf("Удаление вертолёта повлечёт за собой удаление связанных записей!\n");
        printf("Хотите продолжить? ");
        
        if (!get_yes_no()) {
            printf("Операция отменена.\n");
            return 0;
        }
    }
    
    // Начинаем транзакцию
    if (sqlite3_exec(db, "BEGIN TRANSACTION", 0, 0, 0) != SQLITE_OK) {
        printf("Ошибка при начале транзакции: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    
    int success = 1;
    
    // Удаление связанных членов экипажа, если они есть
    if (crew_count > 0) {
        const char *sql_delete_crew = "DELETE FROM Crew_member WHERE helicopter_number = ?";
        sqlite3_stmt *stmt;
        
        if (sqlite3_prepare_v2(db, sql_delete_crew, -1, &stmt, 0) != SQLITE_OK) {
            printf("Ошибка при подготовке запроса удаления членов экипажа: %s\n", sqlite3_errmsg(db));
            success = 0;
        } else {
            sqlite3_bind_int(stmt, 1, helicopter_number);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                printf("Ошибка при удалении членов экипажа: %s\n", sqlite3_errmsg(db));
                success = 0;
            }
            
            sqlite3_finalize(stmt);
        }
    }
    
    // Удаление связанных полётов, если они есть
    if (success && flight_count > 0) {
        const char *sql_delete_flights = "DELETE FROM Flight WHERE helicopter_number = ?";
        sqlite3_stmt *stmt;
        
        if (sqlite3_prepare_v2(db, sql_delete_flights, -1, &stmt, 0) != SQLITE_OK) {
            printf("Ошибка при подготовке запроса удаления полётов: %s\n", sqlite3_errmsg(db));
            success = 0;
        } else {
            sqlite3_bind_int(stmt, 1, helicopter_number);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                printf("Ошибка при удалении полётов: %s\n", sqlite3_errmsg(db));
                success = 0;
            }
            
            sqlite3_finalize(stmt);
        }
    }
    
    // Удаление самого вертолёта
    if (success) {
        const char *sql_delete_helicopter = "DELETE FROM Helicopter WHERE helicopter_number = ?";
        sqlite3_stmt *stmt;
        
        if (sqlite3_prepare_v2(db, sql_delete_helicopter, -1, &stmt, 0) != SQLITE_OK) {
            printf("Ошибка при подготовке запроса удаления вертолёта: %s\n", sqlite3_errmsg(db));
            success = 0;
        } else {
            sqlite3_bind_int(stmt, 1, helicopter_number);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                printf("Ошибка при удалении вертолёта: %s\n", sqlite3_errmsg(db));
                success = 0;
            }
            
            sqlite3_finalize(stmt);
        }
    }
    
    // Завершаем транзакцию
    if (success) {
        if (sqlite3_exec(db, "COMMIT", 0, 0, 0) != SQLITE_OK) {
            printf("Ошибка при завершении транзакции: %s\n", sqlite3_errmsg(db));
            success = 0;
        }
    } else {
        sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
    }
    
    if (success) {
        printf("Вертолёт с номером %d успешно удален", helicopter_number);
        if (crew_count > 0) {
            printf(", а также %d связанных членов экипажа", crew_count);
        }
        if (flight_count > 0) {
            printf(" и %d связанных полётов", flight_count);
        }
        printf(".\n");
        return 0;
    } else {
        printf("Операция удаления отменена из-за ошибок.\n");
        return 1;
    }
}

PilotEarnings retrieve_pilot_earnings(sqlite3 *db, int pilot_id, const char* start_date, const char* end_date) {
    sqlite3_stmt *stmt;
    PilotEarnings earnings = {0};
    earnings.pilot_id = pilot_id;
    earnings.pilot_name = NULL;
    earnings.total_earnings = 0.0;
    earnings.flight_count = 0;
    
    const char *sql =
        "SELECT CM.tab_number, CM.last_name, "
        "COUNT(F.flight_code), "
        "SUM(CASE WHEN F.is_special = 1 THEN F.flight_cost * 0.10 ELSE F.flight_cost * 0.05 END) "
        "FROM Flight F "
        "JOIN Crew_member CM ON F.helicopter_number = CM.helicopter_number "
        "WHERE CM.tab_number = ? "
        "AND F.date BETWEEN ? AND ? "
        "GROUP BY CM.tab_number";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return earnings;
    }
    
    // Привязка параметров
    sqlite3_bind_int(stmt, 1, pilot_id);
    sqlite3_bind_text(stmt, 2, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, end_date, -1, SQLITE_STATIC);
    
    // Выполнение запроса
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        earnings.pilot_id = sqlite3_column_int(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        earnings.pilot_name = name ? strdup(name) : strdup("Неизвестно");
        earnings.flight_count = sqlite3_column_int(stmt, 2);
        earnings.total_earnings = sqlite3_column_double(stmt, 3);
    }
    
    sqlite3_finalize(stmt);
    return earnings;
}

DetailedPilotEarnings retrieve_pilot_earnings_by_flights(sqlite3 *db, int pilot_id, const char* start_date, 
    const char* end_date, int flight_type, int* flights_count) {
sqlite3_stmt *stmt;
DetailedPilotEarnings earnings = {0};
earnings.pilot_id = pilot_id;
earnings.pilot_name = NULL;
earnings.total_earnings = 0.0;
earnings.flight_count = 0;
earnings.flights = NULL;
*flights_count = 0;

// Составление SQL запроса в зависимости от типа рейсов
const char *sql_base = 
"SELECT CM.tab_number, CM.last_name, "
"F.flight_code, F.date, F.is_special, F.flight_cost, "
"CASE WHEN F.is_special = 1 THEN F.flight_cost * 0.10 ELSE F.flight_cost * 0.05 END as pilot_earnings "
"FROM Flight F "
"JOIN Crew_member CM ON F.helicopter_number = CM.helicopter_number "
"WHERE CM.tab_number = ? "
"AND F.date BETWEEN ? AND ? ";

// Добавление условия фильтрации по типу рейса, если указано
const char *sql_all = "";
const char *sql_normal = "AND F.is_special = 0 ";
const char *sql_special = "AND F.is_special = 1 ";
const char *sql_order = "ORDER BY F.date";

// Выбор соответствующего условия фильтрации
const char *sql_filter;
if (flight_type == -1) sql_filter = sql_all;
else if (flight_type == 0) sql_filter = sql_normal;
else sql_filter = sql_special;

// Соединение частей запроса
char sql[512]; // Буфер для полного запроса
snprintf(sql, sizeof(sql), "%s%s%s", sql_base, sql_filter, sql_order);

if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
return earnings;
}

// Привязка параметров
sqlite3_bind_int(stmt, 1, pilot_id);
sqlite3_bind_text(stmt, 2, start_date, -1, SQLITE_STATIC);
sqlite3_bind_text(stmt, 3, end_date, -1, SQLITE_STATIC);

// Первый проход для подсчета количества рейсов
while (sqlite3_step(stmt) == SQLITE_ROW) {
(*flights_count)++;

// Если это первая запись, сохраняем данные о пилоте
if (earnings.pilot_name == NULL) {
earnings.pilot_id = sqlite3_column_int(stmt, 0);
const char* name = (const char*)sqlite3_column_text(stmt, 1);
earnings.pilot_name = name ? strdup(name) : strdup("Неизвестно");
}
}

// Если рейсы не найдены, возвращаем пустую структуру
if (*flights_count == 0) {
sqlite3_finalize(stmt);
return earnings;
}

// Выделяем память для хранения информации о рейсах
earnings.flights = (FlightEarning*)malloc(sizeof(FlightEarning) * (*flights_count));
if (!earnings.flights) {
sqlite3_finalize(stmt);
free(earnings.pilot_name);
earnings.pilot_name = NULL;
fprintf(stderr, "Ошибка выделения памяти\n");
return earnings;
}

// Сброс запроса для повторного выполнения
sqlite3_reset(stmt);

// Второй проход для заполнения данных
int i = 0;
earnings.flight_count = *flights_count;
while (sqlite3_step(stmt) == SQLITE_ROW && i < *flights_count) {
// Данные о рейсе
earnings.flights[i].flight_code = sqlite3_column_int(stmt, 2);

const char* date = (const char*)sqlite3_column_text(stmt, 3);
earnings.flights[i].flight_date = date ? strdup(date) : strdup("N/A");

earnings.flights[i].is_special = sqlite3_column_int(stmt, 4);
earnings.flights[i].flight_cost = sqlite3_column_double(stmt, 5);
earnings.flights[i].pilot_earnings = sqlite3_column_double(stmt, 6);

// Суммируем общий заработок
earnings.total_earnings += earnings.flights[i].pilot_earnings;

i++;
}

sqlite3_finalize(stmt);
return earnings;
}
