-- Vuelo con más asientos vacíos. Muestra en la salida el atributo flight id y el número
-- de asientos vacíos. En caso de empate muestra todos los vuelos que hayan empatado.
-- view that contains all the seats for each aircraft
CREATE OR REPLACE VIEW table_total_seats AS
SELECT
    flight_id,
    Count(*) AS total_seats
FROM
    aircrafts_data
    NATURAL JOIN flights
    NATURAL JOIN seats
GROUP BY
    flight_id;

-- view that has the amount of seats occupied for each plane
CREATE OR REPLACE VIEW occupied_planes AS
SELECT
    flight_id,
    Count(*) AS seats_occupied
FROM
    flights
    NATURAL JOIN ticket_flights
    NATURAL JOIN boarding_passes
WHERE
    status <> 'Cancelled'
GROUP BY
    flight_id;

-- view that only has the empty planes, seast occupied will alway be 0 in this case
CREATE OR REPLACE VIEW empty_planes AS
SELECT
    flight_id,
    0 AS seats_occupied
FROM
    flights
WHERE (status <> 'Cancelled'
    AND NOT EXISTS (
        SELECT
            1
        FROM
            occupied_planes
        WHERE
            flights.flight_id = flight_id));

-- Union empty planes and occupied planes
CREATE OR REPLACE VIEW table_seats_occupied AS
SELECT
    *
FROM
    empty_planes
UNION
SELECT
    *
FROM
    occupied_planes;

-- view which performs the operation to get all free seats in a plane
-- by substracting total_seats with seats_occupied
CREATE OR REPLACE VIEW TABLE_FREE_SEATS AS
SELECT
    flight_id,
    (total_seats - seats_occupied) AS free_seats
FROM
    table_total_seats
    NATURAL JOIN table_seats_occupied;


