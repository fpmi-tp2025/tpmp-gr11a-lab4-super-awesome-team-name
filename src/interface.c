/* interface.c */

#include "interface.h"
#include <stdio.h>

// SELECT
void get_flights_data_by_period(sqlite3 *db);
void get_flights_hours_after_repair(sqlite3 *db);
void get_special_flights_summary(sqlite3 *db);
void get_max_earning_crew(sqlite3 *db);
void get_helicopter_with_most_flights(sqlite3 *db);
void get_normal_flights_summary(sqlite3 *db);

// UPDATE
int update_crew_member(sqlite3 *db);
int update_flight(sqlite3 *db);
int update_helicopter(sqlite3 *db);

// Интерфейс для Commander
void commander_interface(sqlite3 *db) {
    int choice;
    while (1) {
        // Главное меню для командира
        printf("\nВы находитесь в интерфейсе для командира.\n");
        printf("Выберите операцию с базой данных:\n");
        printf("1. SELECT (Просмотр данных)\n");
        printf("2. UPDATE (Обновление данных)\n");
        printf("4. INSERT (Добавление данных)\n");
        printf("3. DELETE (Удаление данных)\n");
        printf("0. Выйти\n");
        printf("Введите ваш выбор: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:  // Если выбрано SELECT
                while (1) {
                    // Подменю для SELECT
                    printf("\nВы выбрали SELECT.\n");
                    printf("1. Получить данные по рейсам в указанном периоде\n");
                    printf("2. Получить налетанные часы и ресурс летного времени после капитального ремонта\n");
                    printf("3. Получить сводку по спецрейсам (общее количество рейсов, масса грузов, заработанные деньги)\n");
                    printf("4. Максимально заработавший экипаж\n");
                    printf("5. Вертолет с макс кол-во рейсов\n");
                    printf("6. Данные по вертолетам, проводившим обычные рейсы\n");
                    printf("0. Вернуться в главное меню\n");
                    printf("Введите ваш выбор: ");
                    scanf("%d", &choice);

                    if (choice == 0) break;  // Выход в главное меню

                    switch (choice) {
                        case 1:
                            get_flights_data_by_period(db);
                            break;
                        case 2:
                            get_flights_hours_after_repair(db);
                            break;
                        case 3:
                            get_special_flights_summary(db);
                            break;
                        case 4:
                            get_max_earning_crew(db);
                            break;
                        case 5:
                            get_helicopter_with_most_flights(db);
                            break;
                        case 6:
                            get_normal_flights_summary(db);
                            break;
                        default:
                            printf("Неверный выбор. Попробуйте снова.\n");
                    }
                }
                break;  // Возврат в главное меню после выхода из подменю SELECT

            case 2:  // Если выбрано UPDATE
                while (1) {
                    printf("\nВы выбрали UPDATE.\n");
                    printf("1. Обновить данные члена экипажа\n");
                    printf("2. Обновить данные о рейсе\n");
                    printf("3. Обновить данные о вертолете\n");
                    printf("0. Вернуться в главное меню\n");
                    printf("Введите ваш выбор: ");
                    scanf("%d", &choice);

                    if (choice == 0) break;  // Выход в главное меню

                    switch (choice) {
                        case 1:
                            update_crew_member(db);
                            break;
                        case 2:
                            update_flight(db);
                            break;
                        case 3:
                            update_helicopter(db);
                        default:
                            printf("Неверный выбор. Попробуйте снова.\n");
                    }
                }
                break;  // Возврат в главное меню после выхода из подменю UPDATE

            case 3: // Если выбрано INSERT
                while (1) {
                    printf("\nВы выбрали INSERT.\n");
                    printf("1. Ничего не реализовано\n");
                    printf("0. Вернуться в главное меню\n");
                    printf("Введите ваш выбор: ");
                    scanf("%d", &choice);

                    if (choice == 0) break;

                    switch (choice) {
                        case 1:
                            printf("Заглушка\n");
                            break;
                        default:
                            printf("Неверный выбор. Попробуйте снова.\n");
                    }
                }
                break;

            case 4:  // Если выбрано DELETE
                while (1) {
                    printf("\nВы выбрали DELETE.\n");
                    printf("1. Ничего не реализовано\n");
                    printf("0. Вернуться в главное меню\n");
                    printf("Введите ваш выбор: ");
                    scanf("%d", &choice);

                    if (choice == 0) break;

                    switch (choice) {
                        case 1:
                            printf("Заглушка\n");
                            break;
                        default:
                            printf("Неверный выбор. Попробуйте снова.\n");
                    }
                }
                break;  // Возврат в главное меню после выхода из подменю DELETE

            case 0:
                printf("Выход из программы.\n");
                return;  // Завершаем программу

            default:
                printf("Неверный выбор. Попробуйте снова.\n");
        }
    }
}

// Интерфейс для Crew Member
void crew_member_interface(sqlite3 *db) {
    printf("Вы находитесь в интерфейсе для члена экипажа.\n");
}
