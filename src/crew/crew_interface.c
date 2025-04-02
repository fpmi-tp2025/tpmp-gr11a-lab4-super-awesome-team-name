/* crew_interface.c */

#include "../../include/crew/crew_interface.h"
#include <stdio.h>

void get_crew_member_info(sqlite3 *db, int tab_number);
void get_helicopter_info(sqlite3 *db, int tab_number);
void get_flight_hours_for_crew_helicopter(sqlite3 *db, int tab_number);
void get_flights_by_period_for_crew(sqlite3 *db, int tab_number);
void calculate_crew_member_earnings(sqlite3 *db, int tab_number, const char *start_date, const char *end_date);
void calculate_crew_member_earnings_for_flight(sqlite3 *db, int tab_number, int flight_code);
void get_all_flights_for_crew(sqlite3 *db, int tab_number);
int update_crew_member_info(sqlite3 *db, int tab_number);

// Интерфейс для Crew Member
void crew_member_interface(sqlite3 *db, int tab_number) {
    printf("Вы находитесь в интерфейсе для члена экипажа.\n");
    int choice;
    char start_date[11], end_date[11];
    int flight_code;
    
    // Основной цикл меню
    while (1) {
        printf("\nВы находитесь в интерфейсе для члена экипажа.\n");
        printf("Выберите операцию:\n");
        printf("1. Просмотреть свою информацию\n");
        printf("2. Просмотреть информацию о вертолете\n");
        printf("3. Посмотреть налетанные часы и ресурс вертолета\n");
        printf("4. Получить данные о рейсах за указанный период\n");
        printf("5. Рассчитать заработок за указанный период\n");
        printf("6. Рассчитать заработок за конкретный рейс\n");
        printf("7. Посмотреть все свои рейсы\n");
        printf("8. Обновить свою личную информацию\n");
        printf("0. Выйти\n");
        printf("Введите ваш выбор: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 0:
                printf("Выход из программы.\n");
                return;
                
            case 1:
                get_crew_member_info(db, tab_number);
                break;
                
            case 2:
                get_helicopter_info(db, tab_number);
                break;
                
            case 3:
                get_flight_hours_for_crew_helicopter(db, tab_number);
                break;
                
            case 4:
                get_flights_by_period_for_crew(db, tab_number);
                break;
                
            case 5:
                printf("Введите дату начала периода (ГГГГ-ММ-ДД): ");
                scanf("%10s", start_date);
                printf("Введите дату конца периода (ГГГГ-ММ-ДД): ");
                scanf("%10s", end_date);
                calculate_crew_member_earnings(db, tab_number, start_date, end_date);
                break;
                
            case 6:
                printf("Введите код рейса: ");
                scanf("%d", &flight_code);
                calculate_crew_member_earnings_for_flight(db, tab_number, flight_code);
                break;
                
            case 7:
                get_all_flights_for_crew(db, tab_number);
                break;
                
            case 8:
                update_crew_member_info(db, tab_number);
                break;
                
            default:
                printf("Неверный выбор. Попробуйте снова.\n");
        }
    }
}