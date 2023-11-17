/*
 Created by roberto on 3/5/21.
*/

#ifndef NCOURSES_SEARCH_H
#define NCOURSES_SEARCH_H
#include "windows.h"
#include <string.h> 
/*#include <unistd.h>*/
void results_search(char * from, char *to, char *date, int ** flights_ids__1, int ** flights_ids__2, int ** num_connections, 
                    int * n_choices, char *** choices, int max_length, int max_rows, WINDOW * window);

void details(int *flights_ids_1, int *flights_ids_2, int *num_connections, WINDOW * window);

#endif /*NCOURSES_SEARCH_H*/
