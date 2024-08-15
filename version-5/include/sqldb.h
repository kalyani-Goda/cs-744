/*
 * This file declares the functions defined by server helper files.
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */
#include "constants.h"

/*
 * This function initializes (creates) the database and returns the lasttoken that was issues.
 * Return: pointer to SQLite database, lasttoken and return value = 0 if success, -1 otherwise.
 */
int sqldb_initialize(sqlite3 **dbPtr, int *lasttoken);

int sqldb_gettoken(sqlite3 *db, int *token);

int sqldb_afterRestart(sqlite3 *db);

int sqldb_insertRequest(sqlite3 *db, int token, int uid, char *filename);

int sqldb_deleteRequest(sqlite3 *db, int token);

int sqldb_updateStatus(sqlite3 *db, int token, int type, char *newValue);

int sqldb_updateTime(sqlite3 *db, int token, double start, double end);

int sqldb_getStatus(sqlite3 *db, int token, int type, char *retValue);

bool sqldb_checkUid(sqlite3 *db, int token, int uid);

void sqldb_finish(sqlite3 *db);

int sqldb_request_status_int(char *result);
char *sqldb_request_status_char(int result);

int sqldb_result_status_int(char *result);
char *sqldb_result_status_char(int result);
