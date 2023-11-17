export PGDATABASE:=flight
export PGUSER :=alumnodb
export PGPASSWORD :=alumnodb
export PGCLIENTENCODING:=UTF8
export PGHOST:=localhost

DBNAME =$(PGDATABASE)
PSQL = psql
CREATEDB = createdb
DROPDB = dropdb --if-exists
PG_DUMP = pg_dump
PG_RESTORE = pg_restore
NUEVABASE = nuevabase

CC = gcc -g
CFLAGS = -Wall -Wextra -pedantic -ansi
LDLIBS = -lodbc -lcurses -lpanel -lmenu -lform

# recompile if this header changes
HEADERS = odbc.h bpass.h  lmenu.h search.h windows.h loop.h odbc.h

EXE = menu
OBJ = $(EXE).o bpass.o   loop.o  search.o windows.o odbc.o

all : dropdb createdb restore query1 $(EXE)

# compile all files ending in *.c
%.o: %.c $(HEADERS)
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c -o $@ $<

# link binary
$(EXE): $(DEPS) $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LDLIBS)

clean:
	rm -f *.o core $(EXE)

run:
	./$(EXE)

run_valgrind:
	valgrind --leak-check=full ./$(EXE)

query1:
	@echo query-1: "please insert here a short description" | tee query1.log
	@cat query4.sql | $(PSQL) | tee -a query4.log

valgrind:
	valgrind --leak-check=full ./$(EXE)

mycomando:
	@echo aqui va una descripcion
	@cat myfichero.sql | psql mibasededatos
createdb:
	@echo Creando BBDD
	@$(CREATEDB)
dropdb:
	@echo Eliminando BBDD
	@$(DROPDB) $(DBNAME)
dump:
	@echo creando dumpfile
	@$(PG_DUMP) > $(DBNAME).sql
restore:
	@echo restore data base
	@cat $(DBNAME).sql | $(PSQL)  
psql: shell
shell:
	@echo create psql shell
	@$(PSQL) 