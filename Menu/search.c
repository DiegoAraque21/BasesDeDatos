/*
* Created by roberto on 3/5/21.
*/
#include "search.h" 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "../odbc-examples/odbc.c"

/* #include "odbc.h" */
void    results_search(char * from, char *to, char *date,
                       int * n_choices, char *** choices,
                       int max_length,
                       int max_rows)
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

    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* using the statement in sql1.txt return me the value */
    char query[2048];
    sprintf(query, "WITH connected_flights AS ( SELECT f1.scheduled_departure AS departure_time_1, f1.departure_airport AS departure_airport_1, f2.scheduled_arrival AS arrival_time_2, f2.arrival_airport AS arrival_airport_2, f1.flight_id AS flight_id_1, f2.flight_id AS flight_id_2, 1 AS num_flights_connection FROM flights f1 JOIN flights f2 ON f1.arrival_airport = f2.departure_airport WHERE f1.departure_airport = 'CEE' AND f2.arrival_airport = 'SVO'AND DATE(f1.scheduled_departure) = DATE('2017-08-14')AND f2.scheduled_arrival < f1.scheduled_departure + INTERVAL '24 hours' AND f2.scheduled_arrival > f1.scheduled_departureAND f2.scheduled_departure > f1.scheduled_arrival), one_way_flights AS ( SELECT  scheduled_departure AS departure_time_1,  departure_airport AS departure_airport_1,  scheduled_arrival AS arrival_time_2,  arrival_airport AS arrival_airport_2,  flight_id AS flight_id_1, CAST(NULL AS integer) AS flight_id_2, 0 AS num_flights_connection FROM flights WHERE departure_airport = 'CEE' AND arrival_airport = 'SVO' AND DATE(scheduled_departure) = DATE('2017-08-14') ), union_table_flights AS ( SELECT * FROM connected_flights UNION SELECT * FROM one_way_flights) SELECT departure_time_1,  departure_airport_1,  arrival_time_2,  arrival_airport_2,  LEAST(frs1.free_seats, frs2.free_seats) AS free_seats,  num_flights_connection, (arrival_time_2 - departure_time_1) as duration FROM union_table_flights t1 JOIN TABLE_FREE_SEATS frs1 ON t1.flight_id_1 = frs1.flight_id LEFT JOIN TABLE_FREE_SEATS frs2 ON t1.flight_id_2 = frs2.flight_id WHERE frs1.free_seats > 0 AND (frs2.free_seats IS NULL OR frs2.free_seats > 0) ORDER BY (arrival_time_2 - departure_time_1) ASC;");

    SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

    SQLBindCol(stmt, 1, SQL_C_CHAR, y, sizeof(y), NULL);

    /* Loop through the rows in the result-set */
    while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
        printf("y = %s\n", y);
    }

    SQLCloseCursor(stmt);

    /* free up statement handle */
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}

