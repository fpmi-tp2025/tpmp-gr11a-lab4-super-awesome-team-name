/* interface.c */

#include "interface.h"
#include <stdio.h>

// Объявления функций, которые уже реализованы в commander.c
void get_flights_data_by_period(sqlite3 *db);
void get_flights_hours_after_repair(sqlite3 *db);
void get_special_flights_summary(sqlite3 *db);
void get_max_earning_crew(sqlite3 *db);
void get_helicopter_with_most_flights(sqlite3 *db);
void get_normal_flights_summary(sqlite3 *db);

// Интерфейс для Commander
void commander_interface(sqlite3 *db) {
    int choice;

    while (1) {
        // Отображение меню для командира
        printf("\nВы находитесь в интерфейсе для командира.\n");
        printf("Выберите действие:\n");
        printf("1. Получить данные по рейсам в указанном периоде\n");
        printf("2. Получить налетанные часы и ресурс летного времени после капитального ремонта\n");
        printf("3. Получить сводку по спецрейсам (общее количество рейсов, масса грузов, заработанные деньги)\n");
        printf("4. Максимально заработавший экипаж\n");
        printf("5. Вертолет с макс кол-во рейсов\n");
        printf("6. Данные по вертолетам проводившие обычные рейсы\n");
        printf("0. Выйти\n");
        printf("Введите ваш выбор: ");
        scanf("%d", &choice);

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
            case 0:
                printf("Выход из программы.\n");
                return;  // Завершаем цикл и выходим из интерфейса
            default:
                printf("Неверный выбор. Попробуйте снова.\n");
        }
    }
}

// Интерфейс для Crew Member
void crew_member_interface(sqlite3 *db) {
    printf("Вы находитесь в интерфейсе для члена экипажа.\n");
}
