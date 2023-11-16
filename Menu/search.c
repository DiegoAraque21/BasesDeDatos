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
                        "       CAST(NULL AS integer) AS flight_id_2, "
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
        while (SQL_SUCCEEDED(SQLFetch(stmt))) {
            (*choices)[*n_choices] = (char*)malloc(max_length * sizeof(char));

            sprintf(result, "%d %d %s %s %s %s %d %d %s", flight_id_1, flight_id_2, departure_time_1, departure_airport_1, arrival_time_2, arrival_airport_2, free_seats, num_flights_connection, duration);

            strcpy((*choices)[*n_choices], result);
            (*n_choices)++;

            write_msg(msg_window, result, -1, -1, "INFO");
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

