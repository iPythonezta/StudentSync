#include <iostream>
#include <crow.h>
#include <sqlite3.h>
#include <jwt-cpp/jwt.h>
using namespace std;

const string secretToken = "e6a76430104f92883b23e707051da61c56172e9276d7b90378b0942d022d4646";

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

bool login(sqlite3* db, string email, string password) {
    string sql = "SELECT email FROM users WHERE email = '" + email + "' AND password = '" + password + "';";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_ROW;
}

void getUserData(sqlite3* db, string email, string password, string& name, bool& isAdmin) {
    string sql = "SELECT name, isAdmin FROM users WHERE email = '" + email + "' AND password = '" + password + "';";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    name = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    isAdmin = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);
}

string generate_token(sqlite3*db, string username, string password,  string secretToken) {
    bool isAdmin;
    string name;
    getUserData(db, username, password, name, isAdmin);
    return jwt::create()
        .set_issuer("StudentSync") 
        .set_payload_claim("username", jwt::claim(username)) 
        .set_payload_claim("role", jwt::claim(string(isAdmin ? "admin" : "user")))
        .sign(jwt::algorithm::hs256(secretToken)); 
}

bool validate_token(string token, string secretToken) {
    try {
        auto decoded_token = jwt::decode(token);
        auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256(secretToken)).with_issuer("StudentSync");
        verifier.verify(decoded_token);
        return true;  
    } catch (const exception& e) {
        return false; 
    }
}

bool decodeToken(string token, string& username, bool& isAdmin) {
    try {
        auto decoded_token = jwt::decode(token);
        username = decoded_token.get_payload_claim("username").as_string();
        isAdmin = decoded_token.get_payload_claim("role").as_string() == "admin";
        return true;  
    } catch (const exception& e) {
        return false; 
    }
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
        password TEXT NOT NULL,
        isAdmin BOOLEAN NOT NULL DEFAULT 0
        );
    )";

    defaultAdminUser(db);

    if (executeSQL(db, create_user)) {
        return -1;
    }

    crow::SimpleApp studentSync;

    CROW_ROUTE(studentSync, "/api/register/")
    .methods("POST"_method)
    ([db](const crow::request& request) {
        string name, email, password;
        bool isAdmin;
        auto json_data = crow::json::load(request.body);
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // remove "Bearer " from the token
        
        string adminUsername;
        bool adminIsAdmin; // true if admin is making the request, false if user is trying to make the request

        if (auth_header.empty()) {
            return crow::response(401, "Authorization token is missing");
        }

        if (!validate_token(auth_header, secretToken)) {
            return crow::response(401, "Invalid token");
        }

        if (!decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "Error decoding token");
        }

        if (!adminIsAdmin) {
            return crow::response(401, "Unauthorized");
        }
        
        if (!json_data) {
            return crow::response(400, "Invalid JSON");
        }
        try{
            name = json_data["name"].s();
            email = json_data["email"].s();
            password = json_data["password"].s();
            isAdmin = json_data["isAdmin"].b();
        }
        catch (const exception& e) {
            return crow::response(400, "Invalid JSON");
        }

        bool userCreated = createUser(db, email, name, password, isAdmin);
        if (userCreated) {
            crow::json::wvalue response;
            response["name"] = name;
            response["email"] = email;
            response["isAdmin"] = isAdmin;
            return crow::response(200, response);
        }
        else {
            return crow::response(400, "User creation failed");
        }
        
    });

    CROW_ROUTE(studentSync, "/api/login/")
    .methods("POST"_method)
    ([db](const crow::request& request) {
        string email, password;
        auto json_data = crow::json::load(request.body);
        if (!json_data) {
            return crow::response(400, "Invalid JSON");
        }
        try{
            email = json_data["email"].s();
            password = json_data["password"].s();
        }
        catch (const exception& e) {
            return crow::response(400, "Invalid JSON");
        }

        if (login(db, email, password)) {
            crow::json::wvalue response;
            response["token"] = generate_token(db, email, password, secretToken);
            return crow::response(200, response);
        }
        else {
            return crow::response(401, "Invalid credentials");
        }

    });

    studentSync.port(2028).run();

}

