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

bool checkUserExxisits(sqlite3* db, string email) {
    string sql = "SELECT email FROM users WHERE email = '" + email + "';";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_ROW;
}

bool createUser(sqlite3* db, string email, string name, string password, bool isAdmin) {
    if (checkUserExxisits(db, email)) {
        cout << "User already exists" << endl;
        return false; // return false if user already exists
    }
    string sql = "INSERT INTO users (email, name, password, isAdmin) VALUES ('" + email + "', '" + name + "', '" + password + "', " + to_string(isAdmin) + ");";
    executeSQL(db, sql.c_str());
    return true; // return true if user is created
}

bool deleteUser(sqlite3* db, string email) {
    if (!checkUserExxisits(db, email)) {
        cout << "User does not exist" << endl;
        return false; // return false if user does not exist
    }
    string sql = "DELETE FROM users WHERE email = '" + email + "';";
    executeSQL(db, sql.c_str());
    return true; // return true if user is deleted
}

bool changePassword(sqlite3* db, string email, string newPassword) {
    if (!checkUserExxisits(db, email)) {
        cout << "User does not exist" << endl;
        return false; // return false if user does not exist
    }
    string sql = "UPDATE users SET password = '" + newPassword + "' WHERE email = '" + email + "';";
    executeSQL(db, sql.c_str());
    return true; // return true if password is changed
}

void defaultAdminUser(sqlite3* db) {
    if (checkUserExxisits(db, "admin")) {
        return;
    }
    createUser(db, "admin", "admin", "admin", true);
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

