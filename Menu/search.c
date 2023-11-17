/*
* Created by roberto on 3/5/21.
*/
#include "search.h" 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
# include "windows.h"

/* #include "odbc.h" */
void    results_search(char * from, char *to, char *date,
                       int ** f_ids_1,
                       int ** f_ids_2,  
                       int ** n_connections,
                       int * n_choices, char *** choices,
                       int max_length,
                       int max_rows,
                       WINDOW * msg_window)
   /**here you need to do your query and fill the choices array of strings
 *
 * @param from form field from
 * @param to form field to
 * @param n_choices fill this with the number of results
 * @param choices fill this with the actual results
 * @param max_length output win maximum width
 * @param max_rows output win maximum number of rows
  */
{
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret; /* ODBC API return status */
    char x[512];
    SQLCHAR y[512];

    /* RESULT VARS */
    SQLCHAR departure_time_1[124], arrival_time_2[124], departure_airport_1[124], arrival_airport_2[124], duration[124];
    SQLINTEGER flight_id_1, flight_id_2, free_seats, num_flights_connection;


    char result[512];

    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        /*extract error*/
        write_msg(msg_window, "Error connecting to database", -1, -1, "ERROR");
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

    SQLPrepare(stmt, (SQLCHAR *) "WITH connected_flights AS ("
                        "SELECT "
                        "f1.scheduled_departure AS departure_time_1, "
                        "f1.departure_airport AS departure_airport_1, "
                        "f2.scheduled_arrival AS arrival_time_2, "
                        "f2.arrival_airport AS arrival_airport_2, "
                        "f1.flight_id AS flight_id_1, "
                        "f2.flight_id AS flight_id_2, "
                        "1 AS num_flights_connection "
                        "FROM flights f1 "
                        "JOIN flights f2 ON f1.arrival_airport = f2.departure_airport "
                        "WHERE f1.departure_airport = ?"
                        "       AND f2.arrival_airport = ? "
                        "       AND DATE(f1.scheduled_departure) = ? "
                        "       AND f2.scheduled_arrival < f1.scheduled_departure + INTERVAL '24 hours' "
                        "       AND f2.scheduled_arrival > f1.scheduled_departure "
                        "       AND f2.scheduled_departure > f1.scheduled_arrival), "
                        "one_way_flights AS ( "
                        "SELECT "
                        "       scheduled_departure AS departure_time_1, "
                        "       departure_airport AS departure_airport_1, "
                        "       scheduled_arrival AS arrival_time_2, "
                        "       arrival_airport AS arrival_airport_2, "
                        "       flight_id AS flight_id_1, "
                        "       0 AS flight_id_2, "
                        "       0 AS num_flights_connection "
                        "FROM flights "
                        "WHERE departure_airport = ? AND arrival_airport = ? "
                        "AND DATE(scheduled_departure) = ? ), "
                        "union_table_flights AS ( "
                        "SELECT * FROM connected_flights "
                        "UNION "
                        "SELECT * FROM one_way_flights "
                        ") "
                        "SELECT "
                        "       flight_id_1, "
                        "       flight_id_2, "
                        "       departure_time_1, "
                        "       departure_airport_1, "
                        "       arrival_time_2, "
                        "       arrival_airport_2, "
                        "       LEAST(frs1.free_seats, frs2.free_seats) AS free_seats, "
                        "       num_flights_connection, "
                        "       (arrival_time_2 - departure_time_1) as duration "
                        "       FROM union_table_flights t1 "
                        "       JOIN TABLE_FREE_SEATS frs1 ON t1.flight_id_1 = frs1.flight_id "
                        "       LEFT JOIN TABLE_FREE_SEATS frs2 ON t1.flight_id_2 = frs2.flight_id "
                        "       WHERE frs1.free_seats > 0 AND (frs2.free_seats IS NULL OR frs2.free_seats > 0) "
                        "       ORDER BY (arrival_time_2 - departure_time_1) ASC", SQL_NTS);

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(from), 0, from, sizeof(from), NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(to), 0, to, sizeof(to), NULL);
    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(date), 0, date, sizeof(date), NULL);
    SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(from), 0, from, sizeof(from), NULL);
    SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(to), 0, to, sizeof(to), NULL);
    SQLBindParameter(stmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(date), 0, date, sizeof(date), NULL);


    if (SQL_SUCCEEDED(SQLExecute(stmt))) {
        /* Bind columns 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 */
        SQLBindCol(stmt, 1, SQL_INTEGER, &flight_id_1, 0, NULL);
        SQLBindCol(stmt, 2, SQL_INTEGER, &flight_id_2, 0, NULL);
        SQLBindCol(stmt, 3, SQL_C_CHAR, departure_time_1, sizeof(departure_time_1), NULL);
        SQLBindCol(stmt, 4, SQL_C_CHAR, departure_airport_1, sizeof(departure_airport_1), NULL);
        SQLBindCol(stmt, 5, SQL_C_CHAR, arrival_time_2, sizeof(arrival_time_2), NULL);
        SQLBindCol(stmt, 6, SQL_C_CHAR, arrival_airport_2, sizeof(arrival_airport_2), NULL);
        SQLBindCol(stmt, 7, SQL_INTEGER, &free_seats, 0, NULL);
        SQLBindCol(stmt, 8, SQL_INTEGER, &num_flights_connection, 0, NULL);
        SQLBindCol(stmt, 9, SQL_C_CHAR, duration, sizeof(duration), NULL);

        /* Loop through the rows in the result-set */
        *n_choices = 0;
        sprintf(result, "Flight_1 Flight_2 Dep_Time Dep_Apt Arr_Time Arr_Apt Seats Num_Flights Duration");
        strcpy((*choices)[*n_choices], result);
        (*n_choices)++;
        while (SQL_SUCCEEDED(SQLFetch(stmt))) {
            (*choices)[*n_choices] = malloc(sizeof(char) * max_length);

            f_ids_1[*n_choices] = (int*)malloc(sizeof(flight_id_1) * 10);
            f_ids_2[*n_choices] = (int*)malloc(sizeof(flight_id_2) * 10);
            n_connections[*n_choices] = (int*)malloc(sizeof(n_connections) * 10);
            sprintf(f_ids_1[*n_choices], "%d", flight_id_1);
            sprintf(f_ids_2[*n_choices], "%d", flight_id_2);
            sprintf(n_connections[*n_choices], "%d", num_flights_connection);

            sprintf(result, "%d %d %s %s %s %s %d %d %s", flight_id_1, flight_id_2, departure_time_1, departure_airport_1, arrival_time_2, arrival_airport_2, free_seats, num_flights_connection, duration);


            strcpy((*choices)[*n_choices], result);
            (*n_choices)++;

        }


        /* close cursor */
        SQLCloseCursor(stmt);

        /* free up statement handle */
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    } else {
        write_msg(msg_window, "Error executing query", -1, -1, "ERROR");
        return EXIT_FAILURE;
    }

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}

void details(int *f_ids_1, int *f_ids_2, int *n_connections, WINDOW * msg_window){
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret; /* ODBC API return status */
    char x[512];
    SQLCHAR y[512];

    char messages[1000];
    char final_message[1000];

    /* RESULT VARS */
    SQLCHAR aircraft_code[124], scheduled_departure[124], scheduled_arrival[124];
    SQLINTEGER flight_id;


    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        /*extract error*/
        write_msg(msg_window, "Error connecting to database", -1, -1, "ERROR");
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

    SQLPrepare(stmt, (SQLCHAR *) "SELECT "
                        "       f.flight_id, "
                        "       f.aircraft_code, "
                        "       f.scheduled_departure, "
                        "       f.scheduled_arrival "
                        "       FROM flights f "
                        "       WHERE f.flight_id = ? OR f.flight_id = ?", SQL_NTS);

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(f_ids_1), 0, f_ids_1, sizeof(f_ids_1), NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(f_ids_2), 0, f_ids_2, sizeof(f_ids_2), NULL);

    if(SQL_SUCCEEDED(SQLExecute(stmt))) {
        /* Bind columns 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 */
        SQLBindCol(stmt, 1, SQL_INTEGER, &flight_id, 0, NULL);
        SQLBindCol(stmt, 2, SQL_C_CHAR, aircraft_code, sizeof(aircraft_code), NULL);
        SQLBindCol(stmt, 3, SQL_C_CHAR, scheduled_departure, sizeof(scheduled_departure), NULL);
        SQLBindCol(stmt, 4, SQL_C_CHAR, scheduled_arrival, sizeof(scheduled_arrival), NULL);

        /* Loop through the rows in the result-set */
        int num = 1;
        while (SQL_SUCCEEDED(SQLFetch(stmt))) {
            sprintf(messages, "Flight %d: %d %s %s %s   ", num, flight_id, aircraft_code, scheduled_departure, scheduled_arrival);
            strcat(final_message, messages);
            num++;
        }

        write_msg(msg_window, final_message, -1, -1, "INFO");

        /* close cursor */
        SQLCloseCursor(stmt);

        /* free up statement handle */
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    } else {
        write_msg(msg_window, "Error executing query", -1, -1, "ERROR");
        return EXIT_FAILURE;
    }

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}

