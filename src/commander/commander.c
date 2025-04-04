/* commander.c */
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/commander/commander.h"

// Функция для получения и данных по рейсам в указанном периоде
FlightPeriodReport* get_flights_report(sqlite3 *db, const char *start, const char *end) {
    // Валидация дат (реализация должна быть предоставлена)
    if (!validate_date(start) || !validate_date(end)) {
        printf("Некорректный формат дат\n");
        return NULL;
    }

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

max_earning_crew_t get_max_earning_crew(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql_crew =
            "SELECT H.helicopter_number, H.model, SUM(F.flight_cost) AS total_earnings "
            "FROM Flight F "
            "JOIN Crew_member CM ON CM.helicopter_number = F.helicopter_number "
            "JOIN Helicopter H ON F.helicopter_number = H.helicopter_number "
            "GROUP BY F.helicopter_number "
            "ORDER BY total_earnings DESC LIMIT 1";

    int rc = sqlite3_prepare_v2(db, sql_crew, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        max_earning_crew_t error_result = {0};
        return error_result; // Возвращаем пустую структуру в случае ошибки
    }

    max_earning_crew_t result = {0};  // Инициализируем структуру

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result.helicopter_number = sqlite3_column_int(stmt, 0);
        const char *helicopter_model = (const char*)sqlite3_column_text(stmt, 1);
        snprintf(result.helicopter_model, sizeof(result.helicopter_model), "%s", helicopter_model);
        result.total_earnings = sqlite3_column_double(stmt, 2);

        // Выводим информацию о вертолете и заработанных деньгах
        printf("Номер вертолета: %d\n", result.helicopter_number);
        printf("Модель вертолета: %s\n", result.helicopter_model);
        printf("Общее количество заработанных денег: %.2f$\n", result.total_earnings);

        const char *sql_crew_members =
                "SELECT CM.tab_number, CM.last_name "
                "FROM Crew_member CM "
                "WHERE CM.helicopter_number = ?";

        sqlite3_stmt *stmt_members;
        rc = sqlite3_prepare_v2(db, sql_crew_members, -1, &stmt_members, 0);
        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return result; // Возвращаем структуру с ошибкой
        }

        sqlite3_bind_int(stmt_members, 1, result.helicopter_number);
        result.crew_count = 0;
        printf("Члены экипажа:\n");
        while (sqlite3_step(stmt_members) == SQLITE_ROW) {
            result.crew_members[result.crew_count].tab_number = sqlite3_column_int(stmt_members, 0);
            const char *last_name = (const char*)sqlite3_column_text(stmt_members, 1);
            snprintf(result.crew_members[result.crew_count].last_name, sizeof(result.crew_members[result.crew_count].last_name), "%s", last_name);

            // Выводим информацию о членах экипажа
            printf("Табельный номер: %d, Фамилия: %s\n", result.crew_members[result.crew_count].tab_number, result.crew_members[result.crew_count].last_name);
            result.crew_count++;
        }

        sqlite3_finalize(stmt_members);

        const char *sql_flights =
                "SELECT F.date, F.flight_code, F.flight_cost "
                "FROM Flight F "
                "WHERE F.helicopter_number = ?";

        sqlite3_stmt *stmt_flights;
        rc = sqlite3_prepare_v2(db, sql_flights, -1, &stmt_flights, 0);
        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return result; // Возвращаем структуру с ошибкой
        }

        sqlite3_bind_int(stmt_flights, 1, result.helicopter_number);
        result.flight_count = 0;
        printf("Рейсы:\n");
        while (sqlite3_step(stmt_flights) == SQLITE_ROW) {
            const char *flight_date = (const char*)sqlite3_column_text(stmt_flights, 0);
            snprintf(result.flights[result.flight_count].date, sizeof(result.flights[result.flight_count].date), "%s", flight_date);
            result.flights[result.flight_count].flight_code = sqlite3_column_int(stmt_flights, 1);
            result.flights[result.flight_count].flight_cost = sqlite3_column_double(stmt_flights, 2);

            // Выводим информацию о рейсах
            printf("Дата: %s, Код полета: %d, Стоимость рейса: %.2f$\n", result.flights[result.flight_count].date, result.flights[result.flight_count].flight_code, result.flights[result.flight_count].flight_cost);
            result.flight_count++;
        }

        sqlite3_finalize(stmt_flights);
    } else {
        printf("Не удалось найти экипаж с максимальными заработками.\n");
    }

    sqlite3_finalize(stmt);
    return result;
}

// Информацию по вертолету и экипажу с макс кол-во рейсов
void get_helicopter_with_most_flights(sqlite3 *db) {
    sqlite3_stmt *stmt;

    // SQL запрос для нахождения вертолета, выполнившего наибольшее количество рейсов
    const char *sql_heli =
            "SELECT H.helicopter_number, H.model, COUNT(F.flight_code) AS num_flights, SUM(F.flight_cost) AS total_earnings "
            "FROM Flight F "
            "JOIN Helicopter H ON F.helicopter_number = H.helicopter_number "
            "GROUP BY F.helicopter_number "
            "ORDER BY num_flights DESC LIMIT 1";  // Получаем вертолет с максимальным количеством рейсов

    int rc = sqlite3_prepare_v2(db, sql_heli, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Извлекаем информацию о вертолете
        int helicopter_number = sqlite3_column_int(stmt, 0);
        const char *helicopter_model = (const char*)sqlite3_column_text(stmt, 1);
        int num_flights = sqlite3_column_int(stmt, 2);
        double total_earnings = sqlite3_column_double(stmt, 3);

        // Выводим информацию о вертолете
        printf("Номер вертолета: %d\n", helicopter_number);
        printf("Модель вертолета: %s\n", helicopter_model);
        printf("Количество рейсов: %d\n", num_flights);
        printf("Количество заработанных денег: %.2f$\n", total_earnings);

        // Получаем информацию о членах экипажа для этого вертолета
        const char *sql_crew =
                "SELECT CM.tab_number, CM.last_name "
                "FROM Crew_member CM "
                "WHERE CM.helicopter_number = ?";  // Получаем членов экипажа для этого вертолета

        sqlite3_stmt *stmt_crew;
        rc = sqlite3_prepare_v2(db, sql_crew, -1, &stmt_crew, 0);
        if (rc != SQLITE_OK) {
            printf("Ошибка при подготовке запроса для членов экипажа: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return;
        }

        sqlite3_bind_int(stmt_crew, 1, helicopter_number);

        // Выводим информацию о экипаже
        printf("Экипаж:\n");

        while (sqlite3_step(stmt_crew) == SQLITE_ROW) {
            int tab_number = sqlite3_column_int(stmt_crew, 0);
            const char *last_name = (const char*)sqlite3_column_text(stmt_crew, 1);
            printf("Табельный номер: %d, Фамилия: %s\n", tab_number, last_name);
        }

        sqlite3_finalize(stmt_crew);
    } else {
        printf("Не удалось найти вертолет с максимальным количеством рейсов.\n");
    }

    sqlite3_finalize(stmt);
}

// Данные по вертолетам проводившие обычный рейс
void get_normal_flights_summary(sqlite3 *db) {
    sqlite3_stmt *stmt;

    // SQL запрос для получения информации о всех вертолетах, выполнявших обычные рейсы
    const char *sql =
            "SELECT H.helicopter_number, H.model, COUNT(F.flight_code) AS num_flights, "
            "SUM(F.cargo_weight) AS total_cargo_weight, SUM(F.flight_cost) AS total_earnings "
            "FROM Flight F "
            "JOIN Helicopter H ON F.helicopter_number = H.helicopter_number "
            "WHERE F.is_special = 0 "  // Отбираем только обычные рейсы (не спецрейсы)
            "GROUP BY F.helicopter_number";  // Группируем по номеру вертолета

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Обработка результатов запроса
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int helicopter_number = sqlite3_column_int(stmt, 0);
        const char *helicopter_model = (const char*)sqlite3_column_text(stmt, 1);
        int num_flights = sqlite3_column_int(stmt, 2);
        double total_cargo_weight = sqlite3_column_double(stmt, 3);
        double total_earnings = sqlite3_column_double(stmt, 4);

        // Вывод информации о каждом вертолете
        printf("Номер вертолета: %d\n", helicopter_number);
        printf("Модель вертолета: %s\n", helicopter_model);
        printf("Количество рейсов: %d\n", num_flights);
        printf("Общая масса перевезенных грузов: %.2f кг\n", total_cargo_weight);
        printf("Общее количество заработанных денег: %.2f$\n", total_earnings);
        printf("\n");
    }

    sqlite3_finalize(stmt);
}

int update_crew_member(sqlite3 *db) {
    int tab_number;
    char field[50];
    char new_value[100];
    int new_helicopter_number;

    // Запрос табельного номера
    printf("Введите табельный номер члена экипажа для обновления: ");
    scanf("%d", &tab_number);

    // Запрос, какое поле нужно изменить
    printf("Какое поле хотите изменить? (last_name, position, birth_year, address, helicopter_number): ");
    scanf("%s", field);

    // Запрос нового значения для этого поля
    printf("Введите новое значение для поля %s: ", field);
    scanf("%s", new_value);

    // Валидация данных в зависимости от поля
    if (strcmp(field, "last_name") == 0) {
        // Проверка фамилии (должна быть строкой)
        if (!validate_name(new_value)) {
            printf("Некорректная фамилия. Используйте только буквы.\n");
            return 1;
        }
    } else if (strcmp(field, "position") == 0) {
        // Проверка должности (должна быть "commander" или "crew_member")
        if (!validate_position(new_value)) {
            printf("Некорректная должность. Должна быть либо 'commander', либо 'crew_member'.\n");
            return 1;
        }
    } else if (strcmp(field, "birth_year") == 0) {
        // Проверка года рождения (должен быть 4 цифры)
        if (!validate_birth_year(new_value)) {
            printf("Некорректный год рождения. Пожалуйста, введите 4 цифры.\n");
            return 1;
        }
    } else if (strcmp(field, "address") == 0) {
        // Адрес не требует сложной валидации в нашем примере
    } else if (strcmp(field, "helicopter_number") == 0) {
        // Проверка наличия вертолета в базе данных
        new_helicopter_number = atoi(new_value);  // Преобразуем строку в целое число
        if (!validate_helicopter_number(db, new_helicopter_number)) {
            printf("Вертолет с таким номером не существует в базе данных.\n");
            return 1;
        }
    } else {
        printf("Некорректное поле.\n");
        return 1;
    }

    // Формирование SQL запроса для обновления данных
    char sql[256];
    snprintf(sql, sizeof(sql), "UPDATE Crew_member SET %s = ? WHERE tab_number = ?", field);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Ошибка при подготовке запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Привязка параметров
    if (strcmp(field, "helicopter_number") == 0) {
        sqlite3_bind_int(stmt, 1, new_helicopter_number);
    } else {
        sqlite3_bind_text(stmt, 1, new_value, -1, SQLITE_STATIC);
    }
    sqlite3_bind_int(stmt, 2, tab_number);

    // Выполнение запроса
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        printf("Ошибка при выполнении запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }

    printf("Данные успешно обновлены.\n");

    // Завершаем работу с запросом
    sqlite3_finalize(stmt);
    return 0;
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
