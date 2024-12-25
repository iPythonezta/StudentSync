#include <iostream>
#include <crow.h>
#include <sqlite3.h>
#include <jwt-cpp/jwt.h>
#include <vector>
#include <regex>
#include <cctype>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using namespace std;
const string secretToken = "e6a76430104f92883b23e707051da61c56172e9276d7b90378b0942d022d4646";

struct Question {
    int id;
    string question;
    string answer;
};

// struct CORS {
//     struct context {};

//     void before_handle(crow::request& req, crow::response& res, context&) {
//         // Handle Preflight Requests (OPTIONS method)
//         if (req.method == crow::HTTPMethod::OPTIONS) {
//             res.add_header("Access-Control-Allow-Origin", "http://localhost:3000"); // Allowed origin
//             res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
//             res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
//             res.code = 204; // No Content
//             res.end();
//         }
//     }

//     void after_handle(crow::request&, crow::response& res, context&) {
//         // Add CORS headers to all responses
//         res.add_header("Access-Control-Allow-Origin", "http://localhost:3000");
//         res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
//         res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
//     }
// };

// Initialization
int executeSQL(sqlite3* db, const char* sql);
void defaultAdminUser(sqlite3* db);
bool login(sqlite3* db, string email, string password);
bool checkUserExxisits(sqlite3* db, string email);

// For the Calender
void getUserData(sqlite3* db, string email, string password, string& name, bool& isAdmin);
void createEvent(sqlite3* db, string title, string description, string schedule_at);
void deleteEvent(sqlite3* db, int id);

// JWT functions
string generate_token(sqlite3* db, string username, string password, string secretToken);
bool decodeToken(string token, string& username, bool& isAdmin);
bool validate_token(string token, string secretToken);

// Users Table Functions
bool createUser(sqlite3* db, string email, string name, string password, bool isAdmin);
bool deleteUser(sqlite3* db, string email);

// Profile function
bool changePassword(sqlite3* db, string email, string newPassword);

// Utility Functions
string toLowerCase(const string& input);
bool matchesIso8601(const string& date);

// For Reading and Writing
bool studentMarksToFile(string userEmail, string assesmentType, string obtainedMarks, string subjectId, string subName, string assesmentId);
int studentMarksFromFile(string userEmail, string assesmentType, int subjectId, string subName, int assesmentId);

// functon for fetching all tasks
vector<vector<string>> getAllEvents(sqlite3* db);
vector<vector<string>> getAllSubjects(sqlite3* db);
vector<vector<string>> getAllQuizes(sqlite3* db, int subject_id);
vector<vector<string>> getAllAssignments(sqlite3* db, int subject_id);
vector<vector<string>> getAllMids(sqlite3* db, int subject_id);
vector<vector<string>> getAllFinals(sqlite3* db, int subject_id);

// Quiz Bank Functions
vector<Question> readCSV(const string& filename);
void writeCSV(const string& filename, const vector<Question>& Questions);
void appendCSV(const string& filename, const Question& Question);
void addQuestion(unordered_map<string, vector<Question>>& subjectMap, const string& subject, int id, const string& question, const string& answer);
void deleteRow(vector<Question>& Questions, int delete_index);
string displayData(const vector<Question>& Questions);

// Group Former Functions
void addSession(string sessionName, int groupMembers, string status);
void deleteSession(int id);
void updateSessionStatus(int id, string status);
vector<vector<string>> readSessions();
string readFile(const string& filename);
void TakePreferences(const string& filename, const string& mail, const vector<string>& preferences);
bool preferenceAlreadyGiven(const string& filename, const string& mail);

bool checkMutualPreference(string email1, string email2, unordered_map<string, vector<string>>& preferences);
void FormGroups(const string filename1, const string filename2, int groupSize, vector<string> emails);

int main(void) {
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

    string create_quizes = R"(
       CREATE TABLE IF NOT EXISTS quizes (
           id INTEGER PRIMARY KEY AUTOINCREMENT,
           subject_id INTEGER NOT NULL,
           quiz_name TEXT NOT NULL,
           quiz_marks INTEGER NOT NULL,
           FOREIGN KEY (subject_id) REFERENCES subject(id)
       )
   )";

    string create_assignments = R"(
       CREATE TABLE IF NOT EXISTS assignments (
           id INTEGER PRIMARY KEY AUTOINCREMENT,
           subject_id INTEGER NOT NULL,
           assignment_name TEXT NOT NULL,
           assignment_marks INTEGER NOT NULL,
           FOREIGN KEY (subject_id) REFERENCES subject(id)
       )
   )";

    string create_mids = R"(
       CREATE TABLE IF NOT EXISTS mids (
           id INTEGER PRIMARY KEY AUTOINCREMENT,
           subject_id INTEGER NOT NULL,
           mid_name TEXT NOT NULL,
           mid_marks INTEGER NOT NULL,
           FOREIGN KEY (subject_id) REFERENCES subject(id)
       )
   )";

    string create_finals = R"(
       CREATE TABLE IF NOT EXISTS finals (
           id INTEGER PRIMARY KEY AUTOINCREMENT,
           subject_id INTEGER NOT NULL,
           final_name TEXT NOT NULL,
           final_marks INTEGER NOT NULL,
           FOREIGN KEY (subject_id) REFERENCES subject(id)
       )    
   )";

    try {
        sqlite3_exec(db, create_user.c_str(), nullptr, 0, nullptr);
        sqlite3_exec(db, create_events.c_str(), nullptr, 0, nullptr);
        sqlite3_exec(db, create_subject.c_str(), nullptr, 0, nullptr);
        sqlite3_exec(db, create_quizes.c_str(), nullptr, 0, nullptr);
        sqlite3_exec(db, create_assignments.c_str(), nullptr, 0, nullptr);
        sqlite3_exec(db, create_mids.c_str(), nullptr, 0, nullptr);
        sqlite3_exec(db, create_finals.c_str(), nullptr, 0, nullptr);
    }
    catch (const exception& e) {
        cout << e.what() << endl;
    }

    defaultAdminUser(db);

    // crow::App<CORS> studentSync;
    crow::SimpleApp studentSync;
    crow::mustache::set_global_base(".");
    // serve home page (React) frontend/build
    CROW_ROUTE(studentSync, "/")
        ([]() {
        auto page = crow::mustache::load("frontend/build/index.html");
        return page.render();
            });

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

        try {
            name = json_data["name"].s();
            email = json_data["email"].s();
            password = json_data["password"].s();
            isAdmin = json_data["isAdmin"].b();
        }
        catch (const exception& e) {
            return crow::response(400, "Invalid JSON");
        }

        email = toLowerCase(email); // Convert email to lowercase
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
        try {
            email = json_data["email"].s();
            password = json_data["password"].s();
        }
        catch (const exception& e) {
            return crow::response(400, "Invalid JSON");
        }

        email = toLowerCase(email); // Convert email to lowercase
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
        ([db](const crow::request& request) {
        string email, name;
        bool isAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, "");
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
        ([db](const crow::request& request) {
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, "");
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "You are not authorized to view this page");
        }
        string sql = "SELECT email, name, isAdmin FROM users;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

        vector<vector<string>> users;
        vector<string> tempUser;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            tempUser.clear();
            tempUser.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
            tempUser.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))));
            tempUser.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))));
            users.push_back(tempUser);
        }
        sqlite3_finalize(stmt);
        vector<crow::json::wvalue> response;
        crow::json::wvalue json_item;

        for (int i = 0; i < users.size(); i++) {
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
        ([db](const crow::request& request) {
        string email;
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, "");
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
        catch (const exception& e) {
            return crow::response(400, "Invalid JSON");
        }

        string sql = "UPDATE users SET isAdmin = 1 WHERE email = '" + email + "';";
        executeSQL(db, sql.c_str());
        return crow::response(200, "User is now an admin");
            });

    CROW_ROUTE(studentSync, "/api/users/remove-user/")
        .methods("DELETE"_method)
        ([db](const crow::request& request) {
        string email;
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, "");
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
        catch (const exception& e) {
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
        ([db](const crow::request& request) {
        string adminUsername, name, datetime, description;
        bool adminIsAdmin;
        if (request.method == "POST"_method) {
            string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, "");
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
                // cout << "Invalid JSON" << endl;
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
            catch (const exception& e) {
                // cout << e.what() << endl;
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
            catch (const exception& e) {
                // cout << e.what() << endl;
                return crow::response(400, "Event creation failed");
            }
        }
        else if (request.method == "GET"_method) {
            string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, "");
            if (auth_header.empty() || !validate_token(auth_header, secretToken)) {
                return crow::response(401, "You must be logged in to view this data");
            }
            vector<vector<string>> events = getAllEvents(db);
            crow::json::wvalue response = crow::json::wvalue::object();
            for (int i = 0; i < events.size(); i++) {
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
        ([db](const crow::request& request, int id) {
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, "");
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
        catch (const exception& e) {
            return crow::response(400, "Error deleting event");
        }
            });

    CROW_ROUTE(studentSync, "/api/subjects/")
        .methods("GET"_method, "POST"_method)
        ([db](const crow::request& request) {
        string adminUsername, name;
        bool adminIsAdmin;
        string sub_name;
        int credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, "");
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
            catch (exception& e) {
                return crow::response(400, "Invalid JSON");
            }
            string sqlStmt = "INSERT INTO subject (name, credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage) VALUES ('" + sub_name + "', " + to_string(credits) + ", " + to_string(quiz_weightage) + ", " + to_string(assignment_weightage) + ", " + to_string(mids_weightage) + ", " + to_string(finals_weightage) + ");";
            try {
                int result = executeSQL(db, sqlStmt.c_str());
                if (result == 0) {
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
            catch (const exception& e) {
                return crow::response(400, "Error creating subject");
            }
        };

        if (request.method == "GET"_method) {

            if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin)) {
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
        ([db](const crow::request& request, int id) {
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, "");
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "You must be logged in to view this data");
        }
        if (request.method == "GET"_method) {

            // ----------------------------------------------------- //

            string sqlQuery = "SELECT id, name, credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage FROM subject WHERE id = " + to_string(id) + ";";
            sqlite3_stmt* stmt;
            crow::json::wvalue subject;
            sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr);
            sqlite3_step(stmt);
            string _id, name, credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage;
            _id = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            name = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            credits = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            quiz_weightage = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
            assignment_weightage = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
            mids_weightage = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
            finals_weightage = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));

            subject["id"] = _id;
            subject["name"] = name;
            subject["credits"] = credits;
            subject["quiz_weightage"] = quiz_weightage;
            subject["assignment_weightage"] = assignment_weightage;
            subject["mids_weightage"] = mids_weightage;
            subject["finals_weightage"] = finals_weightage;

            sqlite3_finalize(stmt);

            // ----------------------------------------------------- //

            vector<vector<string>> quizes = getAllQuizes(db, id);
            vector<vector<string>> assignments = getAllAssignments(db, id);
            vector<vector<string>> mids = getAllMids(db, id);
            vector<vector<string>> finals = getAllFinals(db, id);


            for (int i = 0; i < quizes.size(); i++) {
                subject["quizes"][i] = crow::json::wvalue::object();
                subject["quizes"][i]["quiz_id"] = quizes[i][0];
                subject["quizes"][i]["quiz_name"] = quizes[i][1];
                subject["quizes"][i]["quiz_marks"] = quizes[i][2];
                subject["quizes"][i]["quiz_marks_obtained"] = studentMarksFromFile(adminUsername, "quiz", id, name, stoi(quizes[i][0]));
            }

            for (int i = 0; i < assignments.size(); i++) {
                subject["assignments"][i] = crow::json::wvalue::object();
                subject["assignments"][i]["assignment_id"] = assignments[i][0];
                subject["assignments"][i]["assignment_name"] = assignments[i][1];
                subject["assignments"][i]["assignment_marks"] = assignments[i][2];
                subject["assignments"][i]["assignment_marks_obtained"] = studentMarksFromFile(adminUsername, "assignment", id, name, stoi(assignments[i][0]));
            }

            for (int i = 0; i < mids.size(); i++) {
                subject["mids"][i] = crow::json::wvalue::object();
                subject["mids"][i]["mid_id"] = mids[i][0];
                subject["mids"][i]["mid_name"] = mids[i][1];
                subject["mids"][i]["mid_marks"] = mids[i][2];
                subject["mids"][i]["mid_marks_obtained"] = studentMarksFromFile(adminUsername, "mid", id, name, stoi(mids[i][0]));
            }

            for (int i = 0; i < finals.size(); i++) {
                subject["finals"][i] = crow::json::wvalue::object();
                subject["finals"][i]["final_id"] = finals[i][0];
                subject["finals"][i]["final_name"] = finals[i][1];
                subject["finals"][i]["final_marks"] = finals[i][2];
                subject["finals"][i]["final_marks_obtained"] = studentMarksFromFile(adminUsername, "final", id, name, stoi(finals[i][0]));
            }

            return crow::response(200, subject);
        }

        else if (request.method == "PUT"_method) {
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
            catch (const exception& e) {
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

    CROW_ROUTE(studentSync, "/api/subjects/<int>/add-task/")
        .methods("POST"_method)
        ([db](const crow::request& request, int id) {
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, "");
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "You must be logged in to view this data");
        }
        if (!adminIsAdmin) {
            return crow::response(401, "You are not authorized to perform this action");
        }
        auto json_data = crow::json::load(request.body);
        if (!json_data) {
            return crow::response(400, "Invalid JSON");
        }
        string taskType = json_data["taskType"].s();
        string taskName = json_data["taskName"].s();
        int taskMarks = json_data["taskMarks"].i();
        string sql;
        if (taskType == "quiz") {
            sql = "INSERT INTO quizes (subject_id, quiz_name, quiz_marks) VALUES (" + to_string(id) + ", '" + taskName + "', " + to_string(taskMarks) + ");";
        }
        else if (taskType == "assignment") {
            sql = "INSERT INTO assignments (subject_id, assignment_name, assignment_marks) VALUES (" + to_string(id) + ", '" + taskName + "', " + to_string(taskMarks) + ");";
        }
        else if (taskType == "mid") {
            sql = "INSERT INTO mids (subject_id, mid_name, mid_marks) VALUES (" + to_string(id) + ", '" + taskName + "', " + to_string(taskMarks) + ");";
        }
        else if (taskType == "final") {
            sql = "INSERT INTO finals (subject_id, final_name, final_marks) VALUES (" + to_string(id) + ", '" + taskName + "', " + to_string(taskMarks) + ");";
        }
        else {
            return crow::response(400, "Invalid task type");
        }
        int result = executeSQL(db, sql.c_str());
        if (result == 0) {
            return crow::response(200, "Task added");
        }
        else {
            return crow::response(400, "Error adding task");
        }
            });

    CROW_ROUTE(studentSync, "/api/subjects/activity/<int>/")
        .methods("DELETE"_method)
        ([db](const crow::request& request, int id) {
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, "");
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "You must be logged in to view this data");
        }
        if (!adminIsAdmin) {
            return crow::response(401, "You are not authorized to perform this action");
        }
        auto json_data = crow::json::load(request.body);
        if (!json_data) {
            return crow::response(400, "Invalid JSON");
        }
        string taskType = json_data["taskType"].s();
        string sql;
        if (taskType == "quiz") {
            sql = "DELETE FROM quizes WHERE id = " + to_string(id) + ";";
        }
        else if (taskType == "assignment") {
            sql = "DELETE FROM assignments WHERE id = " + to_string(id) + ";";
        }
        else if (taskType == "mid") {
            sql = "DELETE FROM mids WHERE id = " + to_string(id) + ";";
        }
        else if (taskType == "final") {
            sql = "DELETE FROM finals WHERE id = " + to_string(id) + ";";
        }
        else {
            return crow::response(400, "Invalid task type");
        }
        int result = executeSQL(db, sql.c_str());
        if (result == 0) {
            return crow::response(200, "Task deleted");
        }
        else {
            return crow::response(400, "Error deleting task");
        }
            });

    CROW_ROUTE(studentSync, "/api/change-password/")
        .methods("POST"_method)
        ([db](const crow::request& request) {
        string email, oldPassword, newPassword;

        // Validating the Authorization Token
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Remove "Bearer "
        if (auth_header.empty() || !validate_token(auth_header, secretToken)) {
            return crow::response(401, "Authorization token is missing or invalid");
        }

        // Decode Token to Retrieve User Email
        bool isAdmin;
        if (!decodeToken(auth_header, email, isAdmin)) {
            return crow::response(401, "Failed to decode token");
        }

        // Parsing the JSON Request
        auto json_data = crow::json::load(request.body);
        if (!json_data) {
            return crow::response(400, "Invalid JSON");
        }

        try {
            oldPassword = json_data["oldPassword"].s();
            newPassword = json_data["newPassword"].s();
        }
        catch (const exception& e) {
            return crow::response(400, "Invalid JSON structure");
        }

        // Verifying Old Password
        if (!login(db, email, oldPassword)) {
            return crow::response(401, "Incorrect old password");
        }

        // Updating Password Using `changePassword`
        if (!changePassword(db, email, newPassword)) {
            return crow::response(500, "Failed to update password");
        }

        return crow::response(200, "Password updated successfully");
            });


    CROW_ROUTE(studentSync, "/api/calculate-aggregate/")
        .methods("POST"_method)
        ([db](const crow::request& request) {
        string username;
        bool adminIsAdmin;

        // Authenticate user using the Authorization header
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, "");
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, username, adminIsAdmin)) {
            return crow::response(401, "You must be logged in to view this data");
        }

        // Parse JSON request body
        auto json_data = crow::json::load(request.body);
        if (!json_data) {
            return crow::response(400, "Invalid JSON");
        }

        auto data = json_data["data"];
        if (data.t() != crow::json::type::List || data.size() == 0) {
            return crow::response(400, "Invalid or empty data array");
        }

        // Save student marks to a csv file
        bool result;
        string marks;
        for (size_t i = 0; i < data.size(); i++) {
            try {
                // Extract and format marks
                marks = to_string(data[i]["marks"].d());
            }
            catch (const exception& e) {
                marks = data[i]["marks"].s();
            }

            // Save marks to the file
            result = studentMarksToFile(
                username,
                data[i]["type"].s(),
                marks,
                data[i]["subject_id"].s(),
                data[i]["subject_name"].s(),
                data[i]["task_id"].s()
            );
        }

        // Retrieve weightages from the database
        sqlite3_stmt* stmt;
        double quiz_weightage, assignment_weightage, mids_weightage, finals_weightage;
        string sql = "SELECT quiz_weightage, assignment_weightage, mids_weightage, finals_weightage FROM subject WHERE id = " + string(data[0]["subject_id"].s()) + ";";
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_step(stmt);
        quiz_weightage = sqlite3_column_double(stmt, 0);
        assignment_weightage = sqlite3_column_double(stmt, 1);
        mids_weightage = sqlite3_column_double(stmt, 2);
        finals_weightage = sqlite3_column_double(stmt, 3);
        sqlite3_finalize(stmt);

        // Initialize variables for aggregate calculation
        double totalQuizMarksObtained = 0.0, totalQuizMarks = 0.0;
        double totalAssignmentMarksObtained = 0.0, totalAssignmentMarks = 0.0;
        double totalMidMarksObtained = 0.0, totalMidMarks = 0.0;
        double totalFinalMarksObtained = 0.0, totalFinalMarks = 0.0;

        // Categorize marks by type and calculate totals
        for (int i = 0; i < data.size(); i++) {
            try {
                double marks_;
                try {
                    marks_ = data[i]["marks"].d();
                }
                catch (const exception& e) {
                    marks_ = stod(data[i]["marks"].s());
                }
                double marksObtained = marks_;
                double totalMarksForItem = stod(data[i]["total_marks"].s());
                string type = data[i]["type"].s();
                // e.g., quiz, assignment, mids, finals

                if (type == "quiz") {
                    totalQuizMarksObtained += marksObtained;
                    totalQuizMarks += totalMarksForItem;
                }
                else if (type == "assignment") {
                    totalAssignmentMarksObtained += marksObtained;
                    totalAssignmentMarks += totalMarksForItem;
                }
                else if (type == "mid") {
                    int midMarksFromCSV = studentMarksFromFile(
                        username,
                        "mid",
                        stoi(data[i]["subject_id"].s()),
                        data[i]["subject_name"].s(),
                        stoi(data[i]["task_id"].s())
                    );
                    totalMidMarksObtained += midMarksFromCSV;
                    totalMidMarks += totalMarksForItem;
                }
                else if (type == "final") {
                    int finalMarksFromCSV = studentMarksFromFile(
                        username,
                        "final",
                        stoi(data[i]["subject_id"].s()),
                        data[i]["subject_name"].s(),
                        stoi(data[i]["task_id"].s())
                    );
                    totalFinalMarksObtained += finalMarksFromCSV;
                    totalFinalMarks += totalMarksForItem;
                };
            }
            catch (const exception& e) {
                // cout << e.what() << endl;
                return crow::response(400, "Invalid marks or total_marks format in one or more items.");
            }
        }

        // Calculating weighted aggregates
        double quizAggregate = (totalQuizMarks > 0) ? (totalQuizMarksObtained / totalQuizMarks) * quiz_weightage : 0.0;
        double assignmentAggregate = (totalAssignmentMarks > 0) ? (totalAssignmentMarksObtained / totalAssignmentMarks) * assignment_weightage : 0.0;
        double midAggregate = (totalMidMarks > 0) ? (totalMidMarksObtained / totalMidMarks) * mids_weightage : 0.0;
        double finalAggregate = (totalFinalMarks > 0) ? (totalFinalMarksObtained / totalFinalMarks) * finals_weightage : 0.0;

        // Calculating total aggregate
        double totalAggregate = quizAggregate + assignmentAggregate + midAggregate + finalAggregate;

        // Preparing response
        crow::json::wvalue response;
        response["aggregate"] = totalAggregate;
        response["details"] = {
            {"quizAggregate", quizAggregate},
            {"assignmentAggregate", assignmentAggregate},
            {"midAggregate", midAggregate},
            {"finalAggregate", finalAggregate}
        };
        return crow::response(200, response);
            });



    CROW_ROUTE(studentSync, "/api/quiz/")
        .methods("GET"_method, "POST"_method, "DELETE"_method)
        ([db](const crow::request& req) {

        unordered_map<string, vector<Question>> subjectMap;
        vector<string> subjects;
        string subject;
        string sql = "SELECT name FROM subject";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            subject = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            subjects.push_back(subject);
            subjectMap[subject] =readCSV("quizes/" + subject + ".csv");
        }
        sqlite3_finalize(stmt);
        auto x = crow::json::load(req.body);

        // Handle GET request: Display data for all subjects
        if (req.method == "GET"_method) {
            crow::json::wvalue result;
            for (const auto& subject : subjects) {
                
                result[subject] = displayData(subjectMap[subject]);
            }
            return crow::response{ result };
        }

        // Handle POST request: Add new Question
        if (req.method == "POST"_method && x.has("subject") && x.has("question") && x.has("answer")) {
            string subject = x["subject"].s();
            int id = subjectMap[subject].size() + 1;
            string question = x["question"].s();
            string answer = x["answer"].s();
            addQuestion(subjectMap, subject, id, question, answer);
            return crow::response{ "Question added successfully" };
        }

        // Handle DELETE request: Delete Question
        if (req.method == "DELETE"_method && x.has("subject") && x.has("id")) {
            string subject = x["subject"].s();
            int delete_index = x["id"].i() - 1;
            deleteRow(subjectMap[subject], delete_index);
            writeCSV("quizes/" + subject + ".csv", subjectMap[subject]);
            return crow::response{ "Question deleted successfully" };
        }

        return crow::response{ 400 }; // Bad request if method not handled
    });

    CROW_ROUTE(studentSync, "/api/grouping-sessions/")
    .methods("GET"_method, "POST"_method)
    ([](const crow::request& req) {
        auto x = crow::json::load(req.body);
        
        if (req.method == "GET"_method){
            vector<vector<string>> sessions = readSessions();
            crow::json::wvalue result;
            for (int i=0; i<sessions.size(); i++){
                result[i]["id"] = sessions[i][0];
                result[i]["sessionName"] = sessions[i][1];
                result[i]["membersPerGroup"] = sessions[i][2];
                result[i]["status"] = sessions[i][3];
            }
            return crow::response(200, result);
        }

        if (req.method == "POST"_method){
            string sessionName = x["sessionName"].s();
            int membersPerGroup = x["groupMembers"].i();
            cout << sessionName << " " << membersPerGroup << endl;
            addSession(sessionName, membersPerGroup, "Ongoing");
            return crow::response(200, "Session added successfully");
        }

        
    });

    CROW_ROUTE(studentSync, "/api/grouping-sessions/<int>/")
    .methods("GET"_method, "DELETE"_method, "PUT"_method)
    ([](const crow::request& req, int id) {
        auto x = crow::json::load(req.body);
        string username;
        bool adminIsAdmin;

        string auth_header = string(req.get_header_value("Authorization")).replace(0, 7, "");
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, username, adminIsAdmin)) {
            return crow::response(401, "You must be logged in to view this data");
        }

        if (req.method == "GET"_method){
            vector<vector<string>> sessions = readSessions();
            crow::json::wvalue result;
            for (int i=0; i<sessions.size(); i++){
                if (stoi(sessions[i][0]) == id){
                    result["id"] = sessions[i][0];
                    result["sessionName"] = sessions[i][1];
                    result["membersPerGroup"] = sessions[i][2];
                    result["status"] = sessions[i][3];
                    result["preferenceGiven"] = preferenceAlreadyGiven("GroupFormer/"+sessions[i][1]+"_"+sessions[i][0]+".csv", username);
                }
            }
            return crow::response(200, result);
        }

        if (req.method == "DELETE"_method){
            deleteSession(id);
            return crow::response(200, "Session deleted successfully");
        }
        
        if (req.method == "PUT"_method){
            string status = x["status"].s();
            updateSessionStatus(id, status);
            return crow::response(200, "Session updated successfully");
        }

    });

    CROW_ROUTE(studentSync,"/api/group-preferences/")
    .methods("POST"_method)
    ([](const crow::request& req) {
        auto x = crow::json::load(req.body);
        

        if (req.method == "POST"_method){
            string sessionName = x["sessionName"].s();
            int sessionId = x["sessionId"].i();
            auto preferences = x["preferences"];
            string userEmail = x["email"].s();
            vector<string> preferencesArray;
            for (int i=0; i < preferences.size(); i ++){
                preferencesArray.push_back(preferences[i].s());
            }
            string filename = "GroupFormer/"+sessionName+"_"+to_string(sessionId)+".csv";
            TakePreferences(filename, userEmail, preferencesArray);
            return crow::response(200, "Preferences added successfully");
        }}
    );

    CROW_ROUTE(studentSync,"/api/form-groups/")
    .methods("POST"_method)
    ([db](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (req.method == "POST"_method){
            string sessionName = x["sessionName"].s();
            int sessionId = x["sessionId"].i();
            string filename1 = "GroupFormer/"+sessionName+"_"+to_string(sessionId)+".csv";
            string filename2 = "GroupFormer/"+sessionName+"_"+to_string(sessionId)+"result.csv";
            int membersPerGroup = x["membersPerGroup"].i();
            vector<string> emails;
            string sql = "SELECT email,name FROM users";
            string email,name;
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                email = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
                name = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
                if (email.find(".bese24seecs") != string::npos){
                    emails.push_back(email);
                }
            }
            FormGroups(filename1, filename2, membersPerGroup, emails);
            updateSessionStatus(sessionId, "Completed");
            return crow::response(200, "Group formed successfully");
        }
    });


    CROW_ROUTE(studentSync, "/api/grouping-sessions/groups/")
    .methods("POST"_method)
    ([](const crow::request& req) {
        if (req.method == "POST"_method){
            vector<vector<string>> groups;
            auto x = crow::json::load(req.body);
            string sessionName = x["sessionName"].s();
            int id = x["id"].i();
            string filename = "GroupFormer/"+sessionName+"_"+to_string(id)+"result.csv";
            ifstream file(filename);
            string line;
            string temp;
            vector<string> row;
            while (getline(file, line)){
                for (int i=0; i<line.size(); i++){
                    if (line[i] == ','){
                        row.push_back(temp);
                        temp = "";
                    }
                    else{
                        temp += line[i];
                    }
                }
                groups.push_back(row);
                row.clear();
            }
            crow::json::wvalue result;
            for (int i=0; i<groups.size(); i++){
                result[i] = groups[i];
            }
            return crow::response(200, result);
        }
    });
    studentSync.port(2028).multithreaded().run();

}

bool studentMarksToFile(string userEmail, string assesmentType, string obtainedMarks, string subjectId, string subName, string assesmentId) {
    fstream file;
    string filePath = "student_marks/" + subName + "_" + subjectId + ".csv";
    vector<string> data;
    string line;

    // Open file for reading
    file.open(filePath, ios::in | ios::app);
    if (!file.is_open()) {
        cout << "Error opening file for reading." << endl;
        return false;
    }

    // Read all lines from the file
    while (getline(file, line)) {
        data.push_back(line);
        // cout << line << endl;
    }
    file.close(); // Close after reading

    // Check and edit existing data
    file.open(filePath, ios::out);
    bool done = false;
    for (int i = 0; i < data.size(); i++) {
        if (data[i].find(userEmail) != string::npos && data[i].find(assesmentType) != string::npos && data[i].find(assesmentId) != string::npos) {
            data[i] = userEmail + "," + assesmentType + "," + assesmentId + "," + obtainedMarks;
            done = true;
        }
        // cout << data[i] << endl;
        file << data[i] << endl;
    }

    if (!done) {
        file << userEmail << "," << assesmentType << "," << assesmentId << "," << obtainedMarks << endl;
    }

    file.close();
    return true;
}


int studentMarksFromFile(string userEmail, string assesmentType, int subjectId, string subName, int assesmentId) {
    fstream file;
    string filePath = "student_marks/" + subName + "_" + to_string(subjectId) + ".csv";
    file.open(filePath, ios::in);
    if (!file.is_open()) {
        return 0;
    }
    string line;
    while (getline(file, line)) {
        if (line.find(userEmail) != string::npos && line.find(assesmentType) != string::npos && line.find(to_string(assesmentId)) != string::npos) {
            string marks = line.substr(line.find_last_of(",") + 1);
            return stoi(marks);
        }
    }
    return 0;
}

bool matchesIso8601(const string& date) {
    const regex iso8601Regex(R"(^\d{4}-[01]\d-[0-3]\dT[0-2]\d:[0-5]\d(:[0-5]\d(\.\d+)?([+-][0-2]\d:[0-5]\d|Z)?)?$)");

    return regex_match(date, iso8601Regex);
}

// Function to convert a string to lowercase
string toLowerCase(const string& input) {
    string lowerCaseInput = input; // Create a copy of the input string
    for (size_t i = 0; i < lowerCaseInput.length(); i++) {
        lowerCaseInput[i] = tolower(lowerCaseInput[i]); // Convert each character to lowercase
    }
    return lowerCaseInput;
}

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

string generate_token(sqlite3* db, string username, string password, string secretToken) {
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
    }
    catch (const exception& e) {
        return false;
    }
}

bool decodeToken(string token, string& username, bool& isAdmin) {
    try {
        auto decoded_token = jwt::decode(token);
        username = decoded_token.get_payload_claim("username").as_string();
        isAdmin = decoded_token.get_payload_claim("role").as_string() == "admin";
        return true;
    }
    catch (const exception& e) {
        return false;
    }
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
    email = toLowerCase(email); // Convert email to lowercase
    string sql = "SELECT name, isAdmin FROM users WHERE email = '" + email + "' AND password = '" + password + "';";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    name = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    isAdmin = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);
}

bool checkUserExxisits(sqlite3* db, string email) {
    email = toLowerCase(email); // Convert email to lowercase
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
    string newemail = toLowerCase(email); // Convert email to lowercase
    string sql = "INSERT INTO users (email, name, password, isAdmin) VALUES ('" + newemail + "', '" + name + "', '" + password + "', " + (isAdmin ? "1" : "0") + ");";
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

vector<vector<string>> getAllEvents(sqlite3* db) {
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

vector<vector<string>> getAllSubjects(sqlite3* db) {
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

vector<vector<string>> getAllQuizes(sqlite3* db, int subject_id) {
    vector<vector<string>> quizes;
    string sqlQuery = "SELECT id, quiz_name, quiz_marks FROM quizes WHERE subject_id = " + to_string(subject_id) + ";";
    sqlite3_stmt* stmt;
    vector<string> tempQuiz;
    sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tempQuiz.clear();
        tempQuiz.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
        tempQuiz.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))));
        tempQuiz.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))));
        quizes.push_back(tempQuiz);
    }
    return quizes;
}

vector<vector<string>> getAllAssignments(sqlite3* db, int subject_id) {
    vector<vector<string>> assignments;
    string sqlQuery = "SELECT id, assignment_name, assignment_marks FROM assignments WHERE subject_id = " + to_string(subject_id) + ";";
    sqlite3_stmt* stmt;
    vector<string> tempAssignment;
    sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tempAssignment.clear();
        tempAssignment.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
        tempAssignment.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))));
        tempAssignment.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))));
        assignments.push_back(tempAssignment);
    }
    return assignments;
}

vector<vector<string>> getAllMids(sqlite3* db, int subject_id) {
    vector<vector<string>> mids;
    string sqlQuery = "SELECT id, mid_name, mid_marks FROM mids WHERE subject_id = " + to_string(subject_id) + ";";
    sqlite3_stmt* stmt;
    vector<string> tempMid;
    sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tempMid.clear();
        tempMid.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
        tempMid.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))));
        tempMid.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))));
        mids.push_back(tempMid);
    }
    return mids;
}

vector<vector<string>> getAllFinals(sqlite3* db, int subject_id) {
    vector<vector<string>> finals;
    string sqlQuery = "SELECT id, final_name, final_marks FROM finals WHERE subject_id = " + to_string(subject_id) + ";";
    sqlite3_stmt* stmt;
    vector<string> tempFinal;
    sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tempFinal.clear();
        tempFinal.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
        tempFinal.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))));
        tempFinal.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))));
        finals.push_back(tempFinal);
    }
    return finals;
}

vector<vector<string>> readSessions() {
    fstream file;
    string filePath = "GroupFormer/sessions.csv";
    vector<vector<string>> data;
    vector<string> tempData;
    file.open(filePath, ios::in);  // Open for reading
    string tempLine;
    string temp;
    while (getline(file, tempLine)) {
        temp = "";
        for (int i=0; i<tempLine.length(); i++) {
            if (tempLine[i] == ',') {
                tempData.push_back(temp);
                temp = "";
            }
            else {
                temp += tempLine[i];
            }
        }
        tempData.push_back(temp);
        data.push_back(tempData);
        tempData.clear();

    }
    file.close();
    return data;
}

void addSession(string sessionName, int groupMembers, string status) {
    fstream file;
    string filePath = "GroupFormer/sessions.csv";
    vector<string> data;
    file.open(filePath, ios::in | ios::out); 
    if (!file){
        file.open(filePath, ios::out);
        file.close();
        file.open(filePath, ios::in | ios::out); 
    }
    string tempLine;
    while (getline(file, tempLine)) {
        data.push_back(tempLine);
    }
    file.clear();  
    file.seekg(0, ios::end); 
    file << data.size() + 1 << "," << sessionName << "," << groupMembers << "," << status << endl;  
    file.close();
}

void deleteSession(int id) {
    fstream file;
    string filePath = "GroupFormer/sessions.csv";
    vector<string> data;
    file.open(filePath, ios::in); 
    string tempLine;

    while (getline(file, tempLine)) {
        int commaPos = tempLine.find(',');
        if (commaPos != string::npos) {
            string sessionId = tempLine.substr(0, commaPos); 
            if (sessionId != to_string(id)) { 
                data.push_back(tempLine); 
            }
        } else {
            data.push_back(tempLine);
        }
    }
    file.close();

    file.open(filePath, ios::out | ios::trunc); 
    for (const auto& line : data) {
        file << line << endl;
    }
    file.close();
}


void updateSessionStatus(int id, string status) {
    fstream file;
    string filePath = "GroupFormer/sessions.csv";
    vector<string> data;
    file.open(filePath, ios::in); 
    string tempLine;
    while (getline(file, tempLine)) {
        data.push_back(tempLine);
    }
    file.close();

    file.open(filePath, ios::out | ios::trunc);  
    for (int i = 0; i < data.size(); i++) {
        if (data[i].find(to_string(id)) != string::npos) {
            vector<string> tempData;
            string temp = "";
            for (int j = 0; j < data[i].length(); j++) {
                if (data[i][j] == ',') {
                    tempData.push_back(temp);
                    temp = "";
                } else {
                    temp += data[i][j];
                }
            }
            tempData.push_back(temp);
            data[i] = tempData[0] + "," + tempData[1] + "," + tempData[2] + "," + status;
        }
        file << data[i] << endl;  
    }
    file.close();
}

vector<Question> readCSV(const string& filename) {
    ifstream file(filename);
    string line;
    vector<Question> Questions;

    while (getline(file, line)) {
        int commaPos1 = line.find(',');
        int commaPos2 = line.rfind(',');
        if (commaPos1 != string::npos && commaPos2 != string::npos && commaPos1 != commaPos2) {
            Question Question;
            Question.id = stoi(line.substr(0, commaPos1));
            Question.question = line.substr(commaPos1 + 1, commaPos2 - commaPos1 - 1);
            Question.answer = line.substr(commaPos2 + 1);
            Questions.push_back(Question);
        }
    }
    return Questions;
}

void writeCSV(const string& filename, const vector<Question>& Questions) {
    ofstream file(filename);
    for (const auto& Question : Questions) {
        file << Question.id << "," << Question.question << "," << Question.answer << "\n";
    }
}

void appendCSV(const string& filename, const Question& Question) {
    ofstream file(filename, ios::app);
    file << Question.id << "," << Question.question << "," << Question.answer << "\n";
}

void addQuestion(unordered_map<string, vector<Question>>& subjectMap, const string& subject, int id, const string& question, const string& answer) {
    Question newQuestion = { id, question, answer };
    subjectMap[subject].push_back(newQuestion);
    appendCSV("quizes/" + subject + ".csv", newQuestion);
}

void deleteRow(vector<Question>& Questions, int delete_index) {
    if (delete_index >= 0 && delete_index < Questions.size()) {
        Questions.erase(Questions.begin() + delete_index);
        // Adjust the IDs to be sequential
        for (int i = delete_index; i < Questions.size(); ++i) {
            Questions[i].id = i + 1;
        }
    }
}

string displayData(const vector<Question>& Questions) {
    string result = "ID,Question,Answer\n";
    for (const auto& Question : Questions) {
        result += to_string(Question.id) + "," + Question.question + "," + Question.answer + "\n";
    }
    return result;
}

string readFile(const string& filename) {
    ;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filename << endl;
        return "";
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    return content;
}


void TakePreferences(const string& filename, const string& mail, const vector<string>& preferences) {
    ofstream Outfile(filename, ios::app); // Open the file in append mode

    if (!Outfile.is_open()) {
        cerr << "Error: Could not open the file." << endl;
        return;
    }

    Outfile << mail << ",";
    for (const auto& pref : preferences) {
        Outfile << pref << ",";
    }
    Outfile << "\n";

    Outfile.close(); // Close the file
}

bool preferenceAlreadyGiven(const string& filename, const string& mail) {
    ifstream Infile(filename);
    string line;
    while (getline(Infile, line)) {
        line = toLowerCase(line);
        line = line.substr(0, line.find(','));
        if (line.find(mail) != string::npos) {
            Infile.close();
            return true;
        }
    }
    Infile.close();
    return false;
}

bool checkMutualPreference(string email1, string email2, unordered_map<string, vector<string>>& preferences){
    for (int i=0; i<preferences[email2].size(); i++){
        if(preferences[email2][i] == email1){
            return true;
        }
    }
    return false;
}
void FormGroups(const string filename1, const string filename2, int groupSize, vector<string> emails){
    srand(time(0));
    vector<vector<string>> groups;
    unordered_map<string, vector<string>> preferences;
    unordered_set<string> grouped;
    fstream file;
    file.open(filename1, ios::in);
    string line, tempEmail;

    // Reading preferences from file
    while (getline(file, line)){
        string temp;
        vector<string> row;
        tempEmail = "";
        for (int i=0; i<line.size(); i++){
            if (line[i] == ','){
                if (tempEmail == ""){
                    tempEmail = temp;
                }
                else{
                    row.push_back(temp);
                }
                temp = "";
            }
            else{
                temp += line[i];
            }
        }
        if (temp.size() > 0){
            row.push_back(temp);
        }
        preferences[tempEmail] = row;
    }
    file.close();

    bool mutual;
    vector<string> tempGroup;

    // Initial grouping based on preferences
    for (auto pair:preferences){
        if (grouped.count(pair.first) == 1){
            continue;
        }
        for (int i=0; i < pair.second.size(); i++){
            if (grouped.count(pair.second[i]) == 1){
                continue;
            }
            else {
                mutual = checkMutualPreference(pair.first, pair.second[i], preferences);
                if (mutual){
                    tempGroup.clear();
                    tempGroup.push_back(pair.first);
                    tempGroup.push_back(pair.second[i]);
                    groups.push_back(tempGroup);
                    grouped.insert(pair.first);
                    grouped.insert(pair.second[i]); 
                    break;
                }
            }
        }
    }

    // Fill groups to required size
    for (auto &group:groups){
        if (group.size() >= groupSize){
            continue;
        }
        for (int i=0; i<group.size(); i++){
            auto tempPreferences = preferences[group[i]];
            for (auto preference:tempPreferences){
                if (group.size() >= groupSize){
                    break;
                }
                if (grouped.count(preference)){
                    continue;
                }
                mutual = checkMutualPreference(group[i], preference, preferences);
                if (mutual){
                    group.push_back(preference);
                    grouped.insert(preference); 
                }
            }
            if (group.size() >= groupSize){
                break;
            }
        }
    }

    for (auto &group:groups){
        unordered_map<string, int> emailCount;
        if (group.size() >= groupSize){
            continue;
        }
        for (int i=0; i<group.size(); i++){
            if (group.size() >= groupSize){
                break;
            }
            auto tempPreferences = preferences[group[i]];
            for (auto preference:tempPreferences){
                if (group.size() >= groupSize){
                    break;
                }
                if (grouped.count(preference)){
                    continue;
                }
                if (emailCount.count(preference) == 0){
                    emailCount[preference] = 1;
                }
                else{
                    emailCount[preference]++;
                }
            }
            
        }

        vector<pair<int, string>> sortedEmails;
        for (const auto &entry : emailCount) {
            sortedEmails.push_back({entry.second, entry.first});
        }

        // Sort the emails based on their frequency in descending order
        sort(sortedEmails.rbegin(), sortedEmails.rend());

        // Add the most popular emails to the group until it reaches the required size
        for (auto &email : sortedEmails) {
            if (group.size() >= groupSize) {
                break;
            }
            if (grouped.count(email.second) == 0) {
                group.push_back(email.second);
                grouped.insert(email.second);
            }
        }
    }

    int requiredGroups = ceil((double(emails.size()) / double(groupSize))) - groups.size();
    vector<string> remainingEmails;

    // Collect remaining emails that haven't been grouped
    for (auto email : emails) {
        if (grouped.count(email) == 0) {
            remainingEmails.push_back(email);
        }
    }

    int randEmail;
    for (int i=0; i<requiredGroups; i++){
        if (remainingEmails.empty()) break; 
        randEmail = (rand() % remainingEmails.size());
        tempGroup.clear();
        tempGroup.push_back(remainingEmails[randEmail]);
        grouped.insert(remainingEmails[randEmail]); 
        remainingEmails.erase(remainingEmails.begin()+randEmail);
        groups.push_back(tempGroup);
    }

    // Fill groups to required size
    for (auto &group:groups){
        if (group.size() >= groupSize){
            continue;
        }
        for (int i=0; i<group.size(); i++){
            auto tempPreferences = preferences[group[i]];
            for (auto preference:tempPreferences){
                if (group.size() >= groupSize){
                    break;
                }
                if (grouped.count(preference)){
                    continue;
                }
                mutual = checkMutualPreference(group[i], preference, preferences);
                if (mutual){
                    group.push_back(preference);
                    grouped.insert(preference); 
                }
            }
            if (group.size() >= groupSize){
                break;
            }
        }
    }

    for (auto &group:groups){
        unordered_map<string, int> emailCount;
        if (group.size() >= groupSize){
            continue;
        }
        for (int i=0; i<group.size(); i++){
            if (group.size() >= groupSize){
                break;
            }
            auto tempPreferences = preferences[group[i]];
            for (auto preference:tempPreferences){
                if (group.size() >= groupSize){
                    break;
                }
                if (grouped.count(preference)){
                    continue;
                }
                if (emailCount.count(preference) == 0){
                    emailCount[preference] = 1;
                }
                else{
                    emailCount[preference]++;
                }
            }
            
        }

        vector<pair<int, string>> sortedEmails;
        for (const auto &entry : emailCount) {
            sortedEmails.push_back({entry.second, entry.first});
        }

        // Sort the emails based on their frequency in descending order
        sort(sortedEmails.rbegin(), sortedEmails.rend());

        // Add the most popular emails to the group until it reaches the required size
        for (auto &email : sortedEmails) {
            if (group.size() >= groupSize) {
                break;
            }
            if (grouped.count(email.second) == 0) {
                group.push_back(email.second);
                grouped.insert(email.second);
            }
        }
    }

    for (auto &group:groups){
        if (group.size() >= groupSize){
            continue;
        }
        while (group.size() < groupSize && !remainingEmails.empty()){
            randEmail = (rand() % remainingEmails.size());
            group.push_back(remainingEmails[randEmail]);
            grouped.insert(remainingEmails[randEmail]); 
            remainingEmails.erase(remainingEmails.begin()+randEmail);
        }
    }

    // Output groups to the file
    file.open(filename2, ios::out);
    for(auto &group:groups){
        for (int i=0; i<group.size(); i++){
            file << group[i]<<",";
        }
        file << endl;
    }
}
