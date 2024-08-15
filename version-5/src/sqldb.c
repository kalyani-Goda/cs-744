/*
 * This file is the server helper functions.
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "server_helper.h"

int check_error(int rc) {
	if((rc += SQLITE_OK) || (rc += SQLITE_ROW) || (rc += SQLITE_DONE))
		return SQLITE_OK;
	else 
		return rc;
}
/*
 * This function initializes (creates) the database and returns the lasttoken that was issues.
 * Return: pointer to SQLite database, lasttoken and return value = 0 if success, -1 otherwise.
 */
int sqldb_initialize(sqlite3 **dbPtr, int *lasttoken) 
{

	char* err;
	sqlite3* db;
	sqlite3_open("DB_grader.db", &db);
	char graderTable[]= "CREATE TABLE IF NOT EXISTS autoGraderTable \
    		(requestId INTEGER PRIMARY KEY AUTOINCREMENT, \
		 userId  INTEGER NOT NULL , \
		 filename  TEXT NOT NULL UNIQUE, \
		 requestStatus    TEXT CHECK( requestStatus IN ('N','W','C') )   NOT NULL DEFAULT 'N', \
		 resultStatus    TEXT CHECK( resultStatus IN ('A','P','C', 'E', 'O', 'T') )   NOT NULL DEFAULT 'A', \
		 starttime	REAL, \
		 endtime	REAL \
    		);";

	int rc = sqlite3_exec(db,graderTable, NULL, NULL,&err);
	if(check_error(rc) != SQLITE_OK){
    		printf("error1: %s\n", err);
		sqlite3_free(err);
		return -1;
	}
	sqlite3_free(err);

	rc = sqldb_gettoken(db, lasttoken);
	if(check_error(rc) != SQLITE_OK) {
    		printf("error2: %d\n",rc);
		return -1;
	}
	*dbPtr = db;
	return 0;

}

int callback(void *myarg, int argc, char **argv, char **azColName) {

    sqlite3 *db = (sqlite3 *)myarg;
    if (argc != 2) {
	    printf("expecting 2 colum, got %d\n", argc);
	    return -1;
    }
    int requestId = atoi(argv[0]);
    int userId = atoi(argv[1]);
    add_to_queue(db, requestId, userId, GetTime());

    return 0;
}

int sqldb_afterRestart(sqlite3 *db) {
	char* err;
	char query[] = "SELECT requestId, userId FROM autoGraderTable where requestStatus != 'C'";

	int rc = sqlite3_exec(db, query, callback, (void *)db, &err);
	if(check_error(rc) != SQLITE_OK) {
    		printf("errorR: %s\n", err);
		sqlite3_free(err);
		return -1;
	}
	
	return 0;
}

int sqldb_gettoken(sqlite3 *db, int *token) {
	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db,"SELECT seq from sqlite_sequence where name = 'autoGraderTable'",
		-1,&stmt,0);
	if(check_error(rc) != SQLITE_OK) {
    		printf("error3: %d\n",rc);
		return -1;
	}
	
	rc = sqlite3_step(stmt);
	if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
    		printf("token execution failed: %s\n", sqlite3_errmsg(db));
    		sqlite3_finalize(stmt);
    		return -1;/* failure */;
	}

	*token = sqlite3_column_int(stmt,0);
	sqlite3_finalize(stmt);
	return 0;
}

int sqldb_insertRequest(sqlite3 *db, int token, int uid, char *filename) {
	sqlite3_stmt* stmt;
	char query[]= "INSERT INTO autoGraderTable(requestId, userId, filename) VALUES(?, ?, ?)";
	int rc = sqlite3_prepare_v2(db, query, -1,&stmt,0);
	if(check_error(rc) != SQLITE_OK) {
    		printf("error3: %d\n",rc);
		return -1;
	}
	
	sqlite3_bind_int(stmt, 1, token);
	sqlite3_bind_int(stmt, 2, uid);
	sqlite3_bind_text(stmt, 3, filename, -1, SQLITE_TRANSIENT);
	rc = sqlite3_step(stmt);
	if (check_error(rc) != SQLITE_OK) {
    		printf("insert execution failed: %s\n", sqlite3_errmsg(db));
    		sqlite3_finalize(stmt);
    		return -1;/* failure */;
	}

	sqlite3_finalize(stmt);
	return 0;
}

int sqldb_deleteRequest(sqlite3 *db, int token) {
	sqlite3_stmt* stmt;
	char query[]= "DELETE FROM autoGraderTable where requestId = ?";
	int rc = sqlite3_prepare_v2(db, query, -1,&stmt,0);
	if(check_error(rc) != SQLITE_OK) {
    		printf("error3: %d\n",rc);
		return -1;
	}
	
	sqlite3_bind_int(stmt, 1, token);
	rc = sqlite3_step(stmt);
	if (check_error(rc) != SQLITE_OK) {
    		printf("delete execution failed: %s\n", sqlite3_errmsg(db));
    		sqlite3_finalize(stmt);
    		return -1;/* failure */;
	}

	sqlite3_finalize(stmt);
	return 0;
}

int sqldb_updateStatus(sqlite3 *db, int token, int type, char *newValue) {
	sqlite3_stmt* stmt;
	char *query;
		
	if(type == DB_REQUEST_STATUS) {
		query = "UPDATE autoGraderTable SET requestStatus = ? where requestID = ?";
	}
	else if (type == DB_RESULT_STATUS) {
		query = "UPDATE autoGraderTable SET resultStatus = ? where requestID = ?";
	}
	else {
		printf("unknow update type\n");
		return -1;
	}

	int rc = sqlite3_prepare_v2(db, query, -1,&stmt,0);
	if(check_error(rc) != SQLITE_OK) {
    		printf("error3: %d\n",rc);
		return -1;
	}
	
	sqlite3_bind_text(stmt, 1, newValue, -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, token);
	rc = sqlite3_step(stmt);
	if (check_error(rc) != SQLITE_OK) {
    		printf("status execution failed: %s %d\n", sqlite3_errmsg(db), rc);
    		sqlite3_finalize(stmt);
    		return -1;/* failure */;
	}

	sqlite3_finalize(stmt);
	return 0;
}

int sqldb_updateTime(sqlite3 *db, int token, double start, double end) {
	sqlite3_stmt* stmt;
	char query[] = "UPDATE autoGraderTable SET starttime = ?, endtime = ? where requestID = ?";

	int rc = sqlite3_prepare_v2(db, query, -1,&stmt,0);
	if(check_error(rc) != SQLITE_OK) {
    		printf("error3: %d\n",rc);
		return -1;
	}
	
	sqlite3_bind_double(stmt, 1, start);
	sqlite3_bind_double(stmt, 2, end);
	sqlite3_bind_int(stmt, 3, token);
	rc = sqlite3_step(stmt);
	if (check_error(rc) != SQLITE_OK) {
    		printf("time execution failed: %s\n", sqlite3_errmsg(db));
    		sqlite3_finalize(stmt);
    		return -1;/* failure */;
	}

	sqlite3_finalize(stmt);
	return 0;
}

int sqldb_getStatus(sqlite3 *db, int token, int type, char *retValue) {
	sqlite3_stmt* stmt;
	char *query;
		
	if(type == DB_REQUEST_STATUS) {
		query = "SELECT requestStatus FROM autoGraderTable where requestID = ?";
	}
	else if (type == DB_RESULT_STATUS) {
		query = "SELECT resultStatus FROM autoGraderTable where requestID = ?";
	}
	else {
		printf("unknow update type\n");
		return -1;
	}

	int rc = sqlite3_prepare_v2(db, query, -1,&stmt,0);
	if(check_error(rc) != SQLITE_OK) {
    		printf("error3: %d\n",rc);
		return -1;
	}
	
	sqlite3_bind_int(stmt, 1, token);
	rc = sqlite3_step(stmt);
	if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
    		printf("get execution failed: %s\n", sqlite3_errmsg(db));
    		sqlite3_finalize(stmt);
    		return -1;/* failure */;
	}

	const char *dbret = sqlite3_column_text(stmt,0);
	if(dbret != NULL) {
		strcpy(retValue, dbret);
	}
	sqlite3_finalize(stmt);
	return 0;
}

bool sqldb_checkUid(sqlite3 *db, int token, int uid) {
	sqlite3_stmt* stmt;
	char *query;
		
	query = "SELECT userId FROM autoGraderTable where requestID = ?";

	int rc = sqlite3_prepare_v2(db, query, -1,&stmt,0);
	if(check_error(rc) != SQLITE_OK) {
    		printf("error3: %d\n",rc);
		return false;
	}
	
	sqlite3_bind_int(stmt, 1, token);
	rc = sqlite3_step(stmt);
	if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
    		printf("check uiexecution failed: %s\n", sqlite3_errmsg(db));
    		sqlite3_finalize(stmt);
    		return false;/* failure */;
	}

	int localUid = sqlite3_column_int(stmt,0);
	sqlite3_finalize(stmt);
	printf("rc = %d localuid = %d uid = %d\n", rc, localUid, uid);
	return uid == localUid;
}

void sqldb_finish(sqlite3 *db) {
	if(db != NULL) {
		sqlite3_close(db);
	}
}

int sqldb_request_status_int(char *result) {
	switch(result[0]) {
		case 'C': return  REQUEST_COMPLETED;
		case 'N': return  REQUEST_NEW;
		case 'W': return  REQUEST_WIP;
	}
	return REQUEST_UNKNOWN;
}

char *sqldb_request_status_char(int result) {
	switch(result) {
		case REQUEST_COMPLETED: return "C";
		case REQUEST_NEW: return "N";
		case REQUEST_WIP: return "W";
	}
	return "U";
}

int sqldb_result_status_int(char *result) {
	switch(result[0]) {
		case 'P': return RESULT_PASS;
		case 'C': return RESULT_COMPILE;
		case 'E': return RESULT_ERROR;
		case 'O': return RESULT_OUTPUT_ERROR;
		case 'T': return RESULT_TIMEOUT;
	}
	return RESULT_PENDING;
}

char *sqldb_result_status_char(int result) {
	switch(result) {
		case RESULT_PASS: return "P";
		case RESULT_COMPILE: return "C";
		case RESULT_ERROR: return "E";
		case RESULT_OUTPUT_ERROR: return "O";
		case RESULT_TIMEOUT: return "T";
	}
	return "U";
}
