/*crew_struct.h*/
#ifndef TEAM_LAB4_CREW_STRUCT_H
#define TEAM_LAB4_CREW_STRUCT_H

// get_crew_member_report
typedef struct {
    int tab_number;
    char* last_name;
    char* position;
    int experience_years;
    char* address;
    int birth_year;
    int helicopter_number;
} CrewMember;

// get_helicopter_info
typedef struct {
    int helicopter_number;
    char *model;
    char *manufacture_date;
    double max_payload;
    char *last_repair_date;
    int flight_resource;
    int found;
} HelicopterInfo;

#endif