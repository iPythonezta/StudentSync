#include <iostream>
#include <crow.h>
#include <sqlite3.h>
#include <jwt-cpp/jwt.h>
#include <vector>
#include <regex>

using namespace std;
const string secretToken = "e6a76430104f92883b23e707051da61c56172e9276d7b90378b0942d022d4646";

int executeSQL(sqlite3* db, const char* sql) {
    char* errorMessage;
    int exit = sqlite3_exec(db, sql, nullptr, 0, &errorMessage);

    if (exit != SQLITE_OK) {
        cout << "Error executing SQL: " << errorMessage << endl;
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

bool matchesIso8601(const string& date) {
    const regex iso8601Regex(R"(^\d{4}-[01]\d-[0-3]\dT[0-2]\d:[0-5]\d(:[0-5]\d(\.\d+)?([+-][0-2]\d:[0-5]\d|Z)?)?$)");

    return regex_match(date, iso8601Regex);
}
void createEvent(sqlite3* db, string title, string description, string schedule_at) {
    string sql = "INSERT INTO events (title, description, schedule_at) VALUES ('" + title + "', '" + description + "', '" + schedule_at + "');";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        throw(exception(sqlite3_errmsg(db)));
    }
    sqlite3_finalize(stmt);

}

void deleteEvent(sqlite3* db, int id) {
    string sql = "DELETE FROM events WHERE id = " + to_string(id) + ";";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        throw(exception("Error deleting event"));
    }
}

// functon for fetching all tasks
vector<vector<string>> getAllEvents(sqlite3* db){
    vector<vector<string>> events;
    string sqlQuery = "SELECT id, title, description, schedule_at FROM events;";
    sqlite3_stmt* stmt;
    vector<string> tempEvent;
    sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tempEvent.clear();
        tempEvent.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
        tempEvent.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))));
        tempEvent.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))));
        tempEvent.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))));
        events.push_back(tempEvent);
    }
    return events;
}

vector<vector<string>> getAllSubjects(sqlite3* db){
    vector<vector<string>> subjects;
    string sqlQuery = "SELECT id, name, credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage FROM subject;";
    sqlite3_stmt* stmt;
    vector<string> tempSubject;
    sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {    
        tempSubject.clear();
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))));
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))));
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))));
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))));
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))));
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6))));
        subjects.push_back(tempSubject);
    }
    return subjects;
}

struct CORS {
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context&) {
        // Handle Preflight Requests (OPTIONS method)
        if (req.method == crow::HTTPMethod::OPTIONS) {
            res.add_header("Access-Control-Allow-Origin", "http://localhost:3000"); // Allowed origin
            res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            res.code = 204; // No Content
            res.end();
        }
    }

    void after_handle(crow::request&, crow::response& res, context&) {
        // Add CORS headers to all responses
        res.add_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }
};


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


    string create_user = R"(
        CREATE TABLE IF NOT EXISTS users (
        email TEXT NOT NULL UNIQUE,
        name TEXT NOT NULL,
        password TEXT NOT NULL,
        isAdmin BOOLEAN NOT NULL DEFAULT 0
        );
    )";

    string create_events = R"(
        CREATE TABLE IF NOT EXISTS events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            description TEXT NOT NULL,
            schedule_at TEXT NOT NULL
        )
    )";

    string create_subject = R"(
        CREATE TABLE IF NOT EXISTS subject (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            credits INTEGER NOT NULL,
            quiz_weightage INTEGER NOT NULL,
            assignment_weightage INTEGER NOT NULL,
            mids_weightage INTEGER NOT NULL,
            finals_weightage INTEGER NOT NULL
        )
    )";
    
    try {
        sqlite3_exec(db, create_user.c_str(), nullptr, 0, nullptr);
        sqlite3_exec(db, create_events.c_str(), nullptr, 0, nullptr);
        sqlite3_exec(db, create_subject.c_str(), nullptr, 0, nullptr);
    }
    catch(const exception& e) {
            cout << e.what() << endl;
    }
    
    defaultAdminUser(db);

    crow::App<CORS> studentSync;
    crow::mustache::set_global_base(".");
    // serve home page (React) frontend/build
    CROW_ROUTE(studentSync, "/")
    ([]() {
        auto page = crow::mustache::load("frontend/build/index.html");
        return page.render();
    });

    // static files
    /*CROW_ROUTE(studentSync, "/static/<path>")
    ([](const crow::request& req, crow::response& res, string path) {
        crow::response resp;
        resp.set_static_file_info("frontend/build/static/" + path);
    });*/

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

    CROW_ROUTE(studentSync, "/api/user/") // endpoint to return user data
    .methods("GET"_method)
    ([db](const crow::request& request){
        string email, name;
        bool isAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0,7,"");
        if (auth_header.empty()) {
            return crow::response(401, "Authorization token is missing");
        }

        if (!validate_token(auth_header, secretToken)) {
                return crow::response(401, "Invalid token");
            }

        if (!decodeToken(auth_header, email, isAdmin)) {
            return crow::response(401, "Error decoding token");
        }

        string sqlStatement = "SELECT name, isAdmin FROM users WHERE email = '" + email + "';";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sqlStatement.c_str(), -1, &stmt, nullptr);
        sqlite3_step(stmt);
        name = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        isAdmin = sqlite3_column_int(stmt, 1);
        sqlite3_finalize(stmt);
        crow::json::wvalue response;
        response["name"] = name;
        response["isAdmin"] = isAdmin;
        response["email"] = email;
        return crow::response(200, response);
    });

    CROW_ROUTE(studentSync, "/api/users/")
    .methods("GET"_method)
    ([db](const crow::request& request){
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0,7,"");
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin) || !adminIsAdmin) {
            return crow::response(401, "You are not authorized to view this page");
        }
        string sql = "SELECT email, name, isAdmin FROM users;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

        vector<vector<string>> users;
        vector<string> tempUser;

        while(sqlite3_step(stmt) == SQLITE_ROW) {
            tempUser.clear();
            tempUser.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
            tempUser.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))));
            tempUser.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))));
            users.push_back(tempUser);
        }
        sqlite3_finalize(stmt);
        vector<crow::json::wvalue> response;
        crow::json::wvalue json_item;

        for (int i=0; i<users.size(); i++) {
            json_item["email"] = users[i][0];
            json_item["name"] = users[i][1];
            json_item["isAdmin"] = users[i][2];
            response.push_back(move(json_item));
        }

        crow::json::wvalue final_response;
        final_response["users"] = move(response);

        return crow::response(200, final_response);
    });

    CROW_ROUTE(studentSync, "/api/users/make-admin/")
    .methods("POST"_method)
    ([db](const crow::request& request){
        string email;
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0,7,"");
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin) || !adminIsAdmin) {
            return crow::response(401, "You are not authorized perform this action");
        }

        auto json_data = crow::json::load(request.body);
        if (!json_data) {
            return crow::response(400, "Invalid JSON");
        }

        try {
            email = json_data["email"].s();
        }
        catch(const exception& e) {
            return crow::response(400, "Invalid JSON");
        }

        string sql = "UPDATE users SET isAdmin = 1 WHERE email = '" + email + "';";
        executeSQL(db, sql.c_str());
        return crow::response(200, "User is now an admin");
    });

    CROW_ROUTE(studentSync, "/api/users/remove-user/")
    .methods("DELETE"_method)
    ([db](const crow::request& request){
        string email;
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0,7,"");
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin) || !adminIsAdmin) {
            return crow::response(401, "You are not authorized perform this action");
        }

        auto json_data = crow::json::load(request.body);
        if (!json_data) {
            return crow::response(400, "Invalid JSON");
        }

        try {
            email = json_data["email"].s();
        }
        catch(const exception& e) {
            return crow::response(400, "Invalid JSON");
        }

        bool userDeleted = deleteUser(db, email);
        if (userDeleted) {
            return crow::response(200, "User deleted");
        }
        else {
            return crow::response(400, "User does not exist");
        }
    });

    CROW_ROUTE(studentSync, "/api/events/")
    .methods("GET"_method, "POST"_method)
    ([db](const crow::request& request){
        string adminUsername, name, datetime, description;
        bool adminIsAdmin;
        if (request.method == "POST"_method) {
            string auth_header = string(request.get_header_value("Authorization")).replace(0,7,"");
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

            auto json_data = crow::json::load(request.body);
            if (!json_data) {
                cout << "Invalid JSON" << endl;
                return crow::response(400, "Invalid JSON");
            }

            try {
                name = json_data["name"].s();
                datetime = json_data["dateTime"].s();
                if (!matchesIso8601(datetime)) {
                    return crow::response(400, "Invalid date format");
                }
                description = json_data["description"].s();
            }
            catch(const exception& e) {
                cout << e.what() << endl;
                return crow::response(400, "Invalid JSON");
            }
            try {
                createEvent(db, name, description, datetime);
                crow::json::wvalue response;
                response["name"] = name;
                response["dateTime"] = datetime;
                response["description"] = description;
                return crow::response(200, response);
            }
            catch(const exception& e){
                cout << e.what() << endl;
                return crow::response(400, "Event creation failed");
            }
        }
        else if (request.method == "GET"_method) {
            string auth_header = string(request.get_header_value("Authorization")).replace(0,7,"");
            if (auth_header.empty() || !validate_token(auth_header, secretToken)) {
                return crow::response(401, "You must be logged in to view this data");
            }
            vector<vector<string>> events = getAllEvents(db);
            crow::json::wvalue response = crow::json::wvalue::object();
            for (int i=0; i<events.size(); i++) {
                response[i]["id"] = events[i][0];
                response[i]["name"] = events[i][1];
                response[i]["description"] = events[i][2];
                response[i]["dateTime"] = events[i][3];
            }
            return crow::response(200, response);
        }
    });

    CROW_ROUTE(studentSync, "/api/events/<int>/")
    .methods("DELETE"_method)
    ([db](const crow::request& request, int id){
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0,7,"");
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

        try {
            deleteEvent(db, id);
            return crow::response(200, "Event deleted");
        }
        catch(const exception& e) {
            return crow::response(400, "Error deleting event");
        }
    });

    CROW_ROUTE(studentSync, "/api/subjects/")
    .methods("GET"_method, "POST"_method)
    ([db](const crow::request& request){
        string adminUsername, name;
        bool adminIsAdmin;
        string sub_name;
        int credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage;
        string auth_header = string(request.get_header_value("Authorization")).replace(0,7,"");
        if (request.method == "POST"_method) {
            if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin) || !adminIsAdmin) {
                return crow::response(401, "You are not authorized to perform this action");
            }

            auto json_data = crow::json::load(request.body);
            if (!json_data) {
                return crow::response(400, "Invalid JSON");
            }

            try {
                sub_name = json_data["name"].s();
                credits = json_data["credits"].i();
                quiz_weightage = json_data["quiz_weightage"].i();
                assignment_weightage = json_data["assignment_weightage"].i();
                mids_weightage = json_data["mids_weightage"].i();
                finals_weightage = json_data["finals_weightage"].i();
            }
            catch(exception& e) {
                return crow::response(400, "Invalid JSON");
            }
            string sqlStmt = "INSERT INTO subject (name, credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage) VALUES ('" + sub_name + "', " + to_string(credits) + ", " + to_string(quiz_weightage) + ", " + to_string(assignment_weightage) + ", " + to_string(mids_weightage) + ", " + to_string(finals_weightage) + ");";
            try {
                int result = executeSQL(db, sqlStmt.c_str());
                if (result == 0){
                    crow::json::wvalue response;
                    response["name"] = sub_name;
                    response["credits"] = credits;
                    response["quiz_weightage"] = quiz_weightage;
                    response["assignment_weightage"] = assignment_weightage;
                    response["mids_weightage"] = mids_weightage;
                    response["finals_weightage"] = finals_weightage;
                    return crow::response(200, response);
                }
                else {
                    return crow::response(400, "Error creating subject");
                }
            }
            catch(const exception& e) {
                return crow::response(400, "Error creating subject");
            }
        };

        if (request.method == "GET"_method) {

            if(auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin)) {
                return crow::response(401, "You must be logged in to view this data");
            }

            vector<vector<string>> subjects = getAllSubjects(db);
            crow::json::wvalue response = crow::json::wvalue::object();
            vector<crow::json::wvalue> responseArray;
            crow::json::wvalue json_item;
            for (int i = 0; i < subjects.size(); i++) {
                json_item["id"] = subjects[i][0];
                json_item["name"] = subjects[i][1];
                json_item["credits"] = subjects[i][2];
                json_item["quiz_weightage"] = subjects[i][3];
                json_item["assignment_weightage"] = subjects[i][4];
                json_item["mids_weightage"] = subjects[i][5];
                json_item["finals_weightage"] = subjects[i][6];
                responseArray.push_back(move(json_item));
            }
            response["subjects"] = move(responseArray);
            return crow::response(200, response);
        }
    });

    CROW_ROUTE(studentSync, "/api/subjects/<int>/")
    .methods("GET"_method, "PUT"_method, "DELETE"_method)
    ([db](const crow::request& request, int id){
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0,7,"");
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "You must be logged in to view this data");
        }
        if (request.method == "GET"_method) {
            string sqlQuery = "SELECT id, name, credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage FROM subject WHERE id = " + to_string(id) + ";";
            sqlite3_stmt* stmt;
            crow::json::wvalue subject;
            sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr);
            sqlite3_step(stmt);
            subject["id"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            subject["name"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            subject["credits"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            subject["quiz_weightage"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
            subject["assignment_weightage"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
            subject["mids_weightage"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
            subject["finals_weightage"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));
            sqlite3_finalize(stmt);
            return crow::response(200, subject);
        }
        else if (request.method == "PUT"_method){
            if (!adminIsAdmin) {
                return crow::response(401, "You are not authorized to perform this action");
            }
            auto json_data = crow::json::load(request.body);
            if (!json_data) {
                return crow::response(400, "Invalid JSON");
            }
            string name, credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage;
            try {
                name = json_data["name"].s();
                credits = json_data["credits"].s();
                quiz_weightage = json_data["quiz_weightage"].s();
                assignment_weightage = json_data["assignment_weightage"].s();
                mids_weightage = json_data["mids_weightage"].s();
                finals_weightage = json_data["finals_weightage"].s();
            }
            catch(const exception& e) {
                return crow::response(400, "Invalid JSON");
            }
            string sql = "UPDATE subject SET name = '" + name + "', credits = " + credits + ", quiz_weightage = " + quiz_weightage + ", assignment_weightage = " + assignment_weightage + ", mids_weightage = " + mids_weightage + ", finals_weightage = " + finals_weightage + " WHERE id = " + to_string(id) + ";";
            int result = executeSQL(db, sql.c_str());
            if (result == 0) {
                crow::json::wvalue response;
                response["id"] = id;
                response["name"] = name;
                response["credits"] = credits;
                response["quiz_weightage"] = quiz_weightage;
                response["assignment_weightage"] = assignment_weightage;
                response["mids_weightage"] = mids_weightage;
                response["finals_weightage"] = finals_weightage;
                return crow::response(200, response);
            }
            else {
                return crow::response(400, "Error updating subject");
            }
        }

        else if (request.method == "DELETE"_method) {
            if (!adminIsAdmin) {
                return crow::response(401, "You are not authorized to perform this action");
            }
            string sql = "DELETE FROM subject WHERE id = " + to_string(id) + ";";
            int result = executeSQL(db, sql.c_str());
            if (result == 0) {
                return crow::response(200, "Subject deleted");
            }
            else {
                return crow::response(400, "Error deleting subject");
            }
        }
            
    });

    studentSync.port(2028).run();

}

