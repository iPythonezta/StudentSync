#include <iostream>
#include <crow.h>
#include <sqlite3.h>

using namespace std;

int executeSQL(sqlite3* db, const char* sql) {
    char* errorMessage;
    int exit = sqlite3_exec(db, sql, nullptr, 0, &errorMessage);

    if (exit != SQLITE_OK) {
        cerr << "Error executing SQL: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return 1;
    }
    return 0;
}


int main(void){
    sqlite3* db;
    int exit = sqlite3_open("MainDatabase.db", &db);
    if (exit) {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        return -1;
    }
    else {
        cout << "Database opened successfully!" << endl;
    }

    const char* create_user = R"(
        CREATE TABLE IF NOT EXISTS users (
        email TEXT NOT NULL UNIQUE,
        name TEXT NOT NULL,
        password TEXT NOT NULL
        isAdmin BOOLEAN NOT NULL DEFAULT 0,
        );
    )";

    if (executeSQL(db, create_user)) {
        return -1;
    }

    crow::SimpleApp studentSync;

    CROW_ROUTE(studentSync, "/")([]() {
        return "Hello world";
        });

    studentSync.port(2028).run();

}

