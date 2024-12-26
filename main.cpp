/*
    Program: Student Management System with APIs and Database Integration
    Group Members:
    1 Huzaifa Azhar (TL) 522638
    2 Asim Shoaib 504888
    3 Sheikh Abdullah Bin Zahid 532852

    Description:
    This program implements a student management system with a web API using the Crow framework, SQLite database, and JWT authentication.
    Key functionalities include:
    - User Authentication: Create accounts, login, password management, and role-based authorization with admin-user distinction.
    - Event and task Management: Create, retrieve, and delete scheduled events and tasks.
    - Subject Management: Manage subjects marks and calculate aggregates using set weightages for quizzes, assignments, and exams.
    - Marks Management: update quizzes, assignments, mids, and final exams' marks.
    - Group Formation: Automatically form student groups based on preferences.
    - API Endpoints: Provides a RESTful interface for interacting with the system.
    - File Handling: Stores and retrieves questions and answers, student marks and preferences in CSV files.
    - JWT Integration: Secure token-based user authentication and authorization.

    The program is designed to support student synchronization and academic management workflows for institutions.
*/

#include <iostream>
#include <crow.h>        // Crow framework for creating RESTful APIs and handling HTTP requests and responses.
#include <sqlite3.h>     // SQLite library for managing and interacting with the SQLite database.
#include <jwt-cpp/jwt.h> // JWT (JSON Web Token) library for creating and verifying authentication tokens.
#include <vector>        // Provides a container for managing collections of elements with automatic memory handling and resizing capabilities. 
#include <regex>         // Provides support for regular expressions (ISO 8601 date format validation).
#include <cctype>        // Contains character handling functions (tolower function incorporated in the toLowercase function for case conversion).
#include <sstream>       // Used for string stream operations (constructing SQL queries dynamically).
#include <unordered_map> // Hash table-based associative container for fast unique key and unique  value pair storage (mapping subjects to questions).
#include <unordered_set> // Hash table-based container for unique elements with low average time complexity (tracking grouped emails). 

using namespace std;
// A secret key used for generating and validating JWT tokens to secure API endpoints.
const string secretToken = "e6a76430104f92883b23e707051da61c56172e9276d7b90378b0942d022d4646";

// Represents a question in a quiz bank, including its unique ID, text, and corresponding answer.
struct Question {
    int id;          // Unique identifier for the question.
    string question; // The question text.
    string answer;   // The answer text.
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

// Initialization and database helper functions for setting up users and basic data.
int executeSQL(sqlite3* db, const char* sql);
void defaultAdminUser(sqlite3* db);
bool login(sqlite3* db, string email, string password);
bool checkUserExxisits(sqlite3* db, string email);
void getUserData(sqlite3* db, string email, string password, string& name, bool& isAdmin);

// Calendar management functions for retrieving user data and handling events.
void createEvent(sqlite3* db, string title, string description, string schedule_at);
void deleteEvent(sqlite3* db, int id);
vector<vector<string>> getAllEvents(sqlite3* db);

// functon for fetching all tasks
vector<vector<string>> getAllSubjects(sqlite3* db);
vector<vector<string>> getAllQuizes(sqlite3* db, int subject_id);
vector<vector<string>> getAllAssignments(sqlite3* db, int subject_id);
vector<vector<string>> getAllMids(sqlite3* db, int subject_id);
vector<vector<string>> getAllFinals(sqlite3* db, int subject_id);

// Functions for creating, decoding, and validating JWT tokens for user authentication.
string generate_token(sqlite3* db, string username, string password, string secretToken);
bool decodeToken(string token, string& username, bool& isAdmin);
bool validate_token(string token, string secretToken);

// Functions for managing user records in the database (creation, deletion, and password changes).
bool createUser(sqlite3* db, string email, string name, string password, bool isAdmin);
bool deleteUser(sqlite3* db, string email);

// Function for managing user profile settings (password updates).
bool changePassword(sqlite3* db, string email, string newPassword);

// Helper functions for common operations (string manipulations, date validations).
string toLowerCase(const string& input);
bool matchesIso8601(const string& date);

// Functions for reading from and writing to files (storing student marks and preferences).
bool studentMarksToFile(string userEmail, string assesmentType, string obtainedMarks, string subjectId, string subName, string assesmentId);
int studentMarksFromFile(string userEmail, string assesmentType, int subjectId, string subName, int assesmentId);

// Functions for managing quiz bank data, including reading, writing, adding, and deleting questions.
vector<Question> readCSV(const string& filename);
void writeCSV(const string& filename, const vector<Question>& Questions);
void appendCSV(const string& filename, const Question& Question);
void addQuestion(unordered_map<string, vector<Question>>& subjectMap, const string& subject, int id, const string& question, const string& answer);
void deleteRow(vector<Question>& Questions, int delete_index);
string displayData(const vector<Question>& Questions);

// Functions for forming student groups based on preferences and updating session statuses.
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
    int exit = sqlite3_open("MainDatabase.db", &db);     // Opens the SQLite database named "MainDatabase.db". 
    if (exit) {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;      // If the database cannot be opened, an error message is displayed, and the program exits.
        return -1;
    }
    else {
        cout << "Database opened successfully!" << endl;     // If successful, the database connection is established and ready for use.
    }

    // SQL query to create the "users" table if it does not exist.
    // The table stores user details, including email, name, password, and admin status.
    string create_user = R"(
       CREATE TABLE IF NOT EXISTS users (
       email TEXT NOT NULL UNIQUE,
       name TEXT NOT NULL,
       password TEXT NOT NULL,
       isAdmin BOOLEAN NOT NULL DEFAULT 0
       );
   )";

    // SQL query to create the "events" table if it does not exist.
    // The table stores events with unique IDs, titles, descriptions, and scheduled dates.
    string create_events = R"(
       CREATE TABLE IF NOT EXISTS events (
           id INTEGER PRIMARY KEY AUTOINCREMENT,
           title TEXT NOT NULL,
           description TEXT NOT NULL,
           schedule_at TEXT NOT NULL
       )
   )";

    // SQL query to create the "subject" table if it does not exist.
    // The table stores subject details, including credits and weightages for various assessments.
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

    // SQL query to create the "quizes" table if it does not exist.
    // The table stores quiz details, linked to the corresponding subject using a foreign key.
    string create_quizes = R"(
       CREATE TABLE IF NOT EXISTS quizes (
           id INTEGER PRIMARY KEY AUTOINCREMENT,
           subject_id INTEGER NOT NULL,
           quiz_name TEXT NOT NULL,
           quiz_marks INTEGER NOT NULL,
           FOREIGN KEY (subject_id) REFERENCES subject(id)
       )
   )";

    // SQL query to create the "assignments" table if it does not exist.
    // The table stores assignment details, linked to the corresponding subject using a foreign key.
    string create_assignments = R"(
       CREATE TABLE IF NOT EXISTS assignments (
           id INTEGER PRIMARY KEY AUTOINCREMENT,
           subject_id INTEGER NOT NULL,
           assignment_name TEXT NOT NULL,
           assignment_marks INTEGER NOT NULL,
           FOREIGN KEY (subject_id) REFERENCES subject(id)
       )
   )";

    // SQL query to create the "mids" table if it does not exist.
    // The table stores midterm exam details, linked to the corresponding subject using a foreign key.
    string create_mids = R"(
       CREATE TABLE IF NOT EXISTS mids (
           id INTEGER PRIMARY KEY AUTOINCREMENT,
           subject_id INTEGER NOT NULL,
           mid_name TEXT NOT NULL,
           mid_marks INTEGER NOT NULL,
           FOREIGN KEY (subject_id) REFERENCES subject(id)
       )
   )";

    // SQL query to create the "finals" table if it does not exist.
    // The table stores final exam details, linked to the corresponding subject using a foreign key.
    string create_finals = R"(
       CREATE TABLE IF NOT EXISTS finals (
           id INTEGER PRIMARY KEY AUTOINCREMENT,
           subject_id INTEGER NOT NULL,
           final_name TEXT NOT NULL,
           final_marks INTEGER NOT NULL,
           FOREIGN KEY (subject_id) REFERENCES subject(id)
       )    
   )";

    // Executes the SQL queries to create the necessary tables in the database.
    // Ensures that the required tables are initialized before further operations.
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

    // There should be a default admin user always
    defaultAdminUser(db);

    // crow::App<CORS> studentSync;
    crow::SimpleApp studentSync;
    crow::mustache::set_global_base(".");

    // Endpoint to serve the static homepage of the web application.
    CROW_ROUTE(studentSync, "/")
        ([]() {
        auto page = crow::mustache::load("frontend/build/index.html"); // Loads the compiled frontend HTML file.
        return page.render(); // Renders and returns the HTML to the client.
            });

    CROW_ROUTE(studentSync, "/api/register/") // API endpoint for user registration. Accessible via POST request.
        .methods("POST"_method)
        ([db](const crow::request& request) {
        string name, email, password;
        bool isAdmin;
        auto json_data = crow::json::load(request.body); // Load JSON data from the request body.
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Extract token by removing "Bearer " as every tokes has those first 7 characters.

        string adminUsername;
        bool adminIsAdmin; // Stores if the requester is an admin.

        if (auth_header.empty()) {
            return crow::response(401, "Authorization token is missing"); // Reject if token is not provided.
        }

        if (!validate_token(auth_header, secretToken)) {
            return crow::response(401, "Invalid token"); // Reject if token is invalid.
        }

        if (!decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "Error decoding token"); // Reject if token cannot be decoded.
        }

        if (!adminIsAdmin) {
            return crow::response(401, "Unauthorized"); // Reject if requester is not an admin.
        }

        if (!json_data) {
            return crow::response(400, "Invalid JSON"); // Reject if request body is not valid JSON.
        }

        try {
            name = json_data["name"].s();       // Extract "name" from JSON.
            email = json_data["email"].s();     // Extract "email" from JSON.
            password = json_data["password"].s(); // Extract "password" from JSON.
            isAdmin = json_data["isAdmin"].b(); // Extract "isAdmin" status from JSON.
        }
        catch (const exception& e) {
            return crow::response(400, "Invalid JSON"); // Reject if JSON parsing fails.
        }

        email = toLowerCase(email); // Convert email to lowercase for uniformity.
        bool userCreated = createUser(db, email, name, password, isAdmin); // Attempt to create the user.
        if (userCreated) {
            crow::json::wvalue response;  // Create a success response with user details.
            response["name"] = name;
            response["email"] = email;
            response["isAdmin"] = isAdmin;
            return crow::response(200, response);
        }
        else {
            return crow::response(400, "User creation failed"); // Respond with failure if user creation fails.
        }
            });

    CROW_ROUTE(studentSync, "/api/login/") // API endpoint for user login. Accessible via POST method.
        .methods("POST"_method)
        ([db](const crow::request& request) {
        string email, password; // Variables to store user credentials.

        auto json_data = crow::json::load(request.body); // Parse JSON data from the request body.
        if (!json_data) {
            return crow::response(400, "Invalid JSON"); // Reject if JSON is invalid or missing.
        }

        try {
            email = json_data["email"].s(); // Extract the email from JSON.
            password = json_data["password"].s(); // Extract the password from JSON.
        }
        catch (const exception& e) {
            return crow::response(400, "Invalid JSON"); // Reject if JSON parsing fails.
        }

        email = toLowerCase(email); // Convert email to lowercase for uniformity.

        if (login(db, email, password)) { // Verify the user's credentials using the database.
            crow::json::wvalue response; // Prepare JSON response.
            response["token"] = generate_token(db, email, password, secretToken); // Generate a JWT token for the user.
            return crow::response(200, response); // Return the token as a success response.
        }
        else {
            return crow::response(401, "Invalid credentials"); // Respond with an error if login fails.
        }
            });

    CROW_ROUTE(studentSync, "/api/user/") // API endpoint to retrieve the logged-in user's details. Accessible via GET request.
        .methods("GET"_method)
        ([db](const crow::request& request) {
        string email, name;
        bool isAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Extract token by removing "Bearer ".

        if (auth_header.empty()) {
            return crow::response(401, "Authorization token is missing"); // Reject if token is not provided.
        }

        if (!validate_token(auth_header, secretToken)) {
            return crow::response(401, "Invalid token"); // Reject if token is invalid.
        }

        if (!decodeToken(auth_header, email, isAdmin)) {
            return crow::response(401, "Error decoding token"); // Reject if token decoding fails.
        }

        string sqlStatement = "SELECT name, isAdmin FROM users WHERE email = '" + email + "';";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sqlStatement.c_str(), -1, &stmt, nullptr); // Prepare SQL to fetch user's name and admin status.
        sqlite3_step(stmt);
        name = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))); // Extract the user's name.
        isAdmin = sqlite3_column_int(stmt, 1); // Extract admin status (0 or 1).
        sqlite3_finalize(stmt);

        crow::json::wvalue response; // Prepare response JSON.
        response["name"] = name;
        response["isAdmin"] = isAdmin;
        response["email"] = email;
        return crow::response(200, response); // Return user details in JSON format.
            });

    CROW_ROUTE(studentSync, "/api/users/") // API endpoint to retrieve all user details. Accessible via GET request.
        .methods("GET"_method)
        ([db](const crow::request& request) {
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Extract token by removing "Bearer ".

        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "You are not authorized to view this page"); // Reject unauthorized requests.
        }

        string sql = "SELECT email, name, isAdmin FROM users;"; // SQL query to fetch all users' details.
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr); // Prepare SQL statement.

        vector<vector<string>> users; // Container to store user records.
        vector<string> tempUser;

        while (sqlite3_step(stmt) == SQLITE_ROW) { // Iterate over the result rows.
            tempUser.clear();
            tempUser.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)))); // Fetch email.
            tempUser.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)))); // Fetch name.
            tempUser.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)))); // Fetch admin status.
            users.push_back(tempUser);
        }
        sqlite3_finalize(stmt); // Finalize the prepared statement.

        vector<crow::json::wvalue> response; // Container for the JSON response.
        crow::json::wvalue json_item;

        for (int i = 0; i < users.size(); i++) { // Populate the JSON response with user details.
            json_item["email"] = users[i][0];
            json_item["name"] = users[i][1];
            json_item["isAdmin"] = users[i][2];
            response.push_back(move(json_item));
        }

        crow::json::wvalue final_response;
        final_response["users"] = move(response); // Add user list to the final JSON response.

        return crow::response(200, final_response); // Return the response containing all user details.
            });

    CROW_ROUTE(studentSync, "/api/users/make-admin/") // API endpoint to promote a user to an admin role. Accessible via POST request.
        .methods("POST"_method)
        ([db](const crow::request& request) {
        string email; // Email of the user to be promoted.
        string adminUsername; // Username of the admin making the request.
        bool adminIsAdmin; // Indicates if the requester is an admin.

        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Extract token by removing "Bearer ".
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin) || !adminIsAdmin) {
            return crow::response(401, "You are not authorized to perform this action"); // Reject unauthorized requests.
        }

        auto json_data = crow::json::load(request.body); // Parse JSON data from the request body.
        if (!json_data) {
            return crow::response(400, "Invalid JSON"); // Reject if JSON is invalid or missing.
        }

        try {
            email = json_data["email"].s(); // Extract the email of the user to be promoted.
        }
        catch (const exception& e) {
            return crow::response(400, "Invalid JSON"); // Reject if email extraction fails.
        }

        string sql = "UPDATE users SET isAdmin = 1 WHERE email = '" + email + "';"; // SQL query to update the user's admin status.
        executeSQL(db, sql.c_str()); // Execute the SQL query.
        return crow::response(200, "User is now an admin"); // Respond with a success message.
            });

    CROW_ROUTE(studentSync, "/api/users/remove-user/") // API endpoint to delete a user from the system. Accessible via DELETE request.
        .methods("DELETE"_method)
        ([db](const crow::request& request) {
        string email; // Email of the user to be deleted.
        string adminUsername; // Username of the admin making the request.
        bool adminIsAdmin; // Indicates if the requester is an admin.

        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Extract token by removing "Bearer ".
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin) || !adminIsAdmin) {
            return crow::response(401, "You are not authorized to perform this action"); // Reject unauthorized requests.
        }

        auto json_data = crow::json::load(request.body); // Parse JSON data from the request body.
        if (!json_data) {
            return crow::response(400, "Invalid JSON"); // Reject if JSON is invalid or missing.
        }

        try {
            email = json_data["email"].s(); // Extract the email of the user to be deleted.
        }
        catch (const exception& e) {
            return crow::response(400, "Invalid JSON"); // Reject if email extraction fails.
        }

        bool userDeleted = deleteUser(db, email); // Attempt to delete the user from the database.
        if (userDeleted) {
            return crow::response(200, "User deleted"); // Respond with success if the user was deleted.
        }
        else {
            return crow::response(400, "User does not exist"); // Respond with an error if the user does not exist.
        }
            });

    CROW_ROUTE(studentSync, "/api/events/") // API endpoint to manage events. Accessible via GET and POST methods.
        .methods("GET"_method, "POST"_method)
        ([db](const crow::request& request) {
        string adminUsername, name, datetime, description;
        bool adminIsAdmin;

        // Handle POST requests for creating a new event.
        if (request.method == "POST"_method) {
            string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Extract token by removing "Bearer ".
            if (auth_header.empty()) {
                return crow::response(401, "Authorization token is missing"); // Reject if token is missing.
            }
            if (!validate_token(auth_header, secretToken)) {
                return crow::response(401, "Invalid token"); // Reject if token is invalid.
            }
            if (!decodeToken(auth_header, adminUsername, adminIsAdmin)) {
                return crow::response(401, "Error decoding token"); // Reject if token decoding fails.
            }
            if (!adminIsAdmin) {
                return crow::response(401, "Unauthorized"); // Reject if the user is not an admin.
            }

            auto json_data = crow::json::load(request.body); // Parse JSON data from the request body.
            if (!json_data) {
                return crow::response(400, "Invalid JSON"); // Reject if JSON is invalid or missing.
            }

            try {
                name = json_data["name"].s(); // Extract the event name.
                datetime = json_data["dateTime"].s(); // Extract the event date and time.
                if (!matchesIso8601(datetime)) { // Validate the date format.
                    return crow::response(400, "Invalid date format");
                }
                description = json_data["description"].s(); // Extract the event description.
            }
            catch (const exception& e) {
                return crow::response(400, "Invalid JSON"); // Reject if JSON parsing fails.
            }

            try {
                createEvent(db, name, description, datetime); // Create the event in the database.
                crow::json::wvalue response; // Prepare success response.
                response["name"] = name;
                response["dateTime"] = datetime;
                response["description"] = description;
                return crow::response(200, response); // Return success response with event details.
            }
            catch (const exception& e) {
                return crow::response(400, "Event creation failed"); // Respond with an error if event creation fails.
            }
        }

        // Handle GET requests for retrieving all events.
        else if (request.method == "GET"_method) {
            string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Extract token by removing "Bearer ".
            if (auth_header.empty() || !validate_token(auth_header, secretToken)) {
                return crow::response(401, "You must be logged in to view this data"); // Reject unauthorized requests.
            }

            vector<vector<string>> events = getAllEvents(db); // Fetch all events from the database.
            crow::json::wvalue response = crow::json::wvalue::object(); // Prepare JSON response.
            for (int i = 0; i < events.size(); i++) {
                response[i]["id"] = events[i][0]; // Event ID.
                response[i]["name"] = events[i][1]; // Event name.
                response[i]["description"] = events[i][2]; // Event description.
                response[i]["dateTime"] = events[i][3]; // Event date and time.
            }
            return crow::response(200, response); // Return response with all event details.
        }
            });

    CROW_ROUTE(studentSync, "/api/events/<int>/") // API endpoint to delete a specific event. Accessible via DELETE method.
        .methods("DELETE"_method)
        ([db](const crow::request& request, int id) {
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Extract token by removing "Bearer ".
        if (auth_header.empty()) {
            return crow::response(401, "Authorization token is missing"); // Reject if token is missing.
        }
        if (!validate_token(auth_header, secretToken)) {
            return crow::response(401, "Invalid token"); // Reject if token is invalid.
        }
        if (!decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "Error decoding token"); // Reject if token decoding fails.
        }
        if (!adminIsAdmin) {
            return crow::response(401, "Unauthorized"); // Reject if the user is not an admin.
        }

        try {
            deleteEvent(db, id); // Attempt to delete the event with the given ID from the database.
            return crow::response(200, "Event deleted"); // Respond with success if the event is deleted.
        }
        catch (const exception& e) {
            return crow::response(400, "Error deleting event"); // Respond with an error if event deletion fails.
        }
            });

    CROW_ROUTE(studentSync, "/api/subjects/") // API endpoint to manage subjects. Accessible via GET and POST methods.
        .methods("GET"_method, "POST"_method)
        ([db](const crow::request& request) {
        string adminUsername, name;
        bool adminIsAdmin;
        string sub_name;
        int credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Extract token by removing "Bearer ".

        // Handle POST requests for creating a new subject.
        if (request.method == "POST"_method) {
            if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin) || !adminIsAdmin) {
                return crow::response(401, "You are not authorized to perform this action"); // Reject unauthorized requests.
            }

            auto json_data = crow::json::load(request.body); // Parse JSON data from the request body.
            if (!json_data) {
                return crow::response(400, "Invalid JSON"); // Reject if JSON is invalid or missing.
            }

            try {
                sub_name = json_data["name"].s(); // Extract subject name.
                credits = json_data["credits"].i(); // Extract subject credits.
                quiz_weightage = json_data["quiz_weightage"].i(); // Extract quiz weightage.
                assignment_weightage = json_data["assignment_weightage"].i(); // Extract assignment weightage.
                mids_weightage = json_data["mids_weightage"].i(); // Extract midterm weightage.
                finals_weightage = json_data["finals_weightage"].i(); // Extract final exam weightage.
            }
            catch (exception& e) {
                return crow::response(400, "Invalid JSON"); // Reject if JSON parsing fails.
            }

            string sqlStmt = "INSERT INTO subject (name, credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage) VALUES ('"
                + sub_name + "', " + to_string(credits) + ", " + to_string(quiz_weightage) + ", " + to_string(assignment_weightage)
                + ", " + to_string(mids_weightage) + ", " + to_string(finals_weightage) + ");";

            try {
                int result = executeSQL(db, sqlStmt.c_str()); // Execute the SQL query to create a new subject.
                if (result == 0) {
                    crow::json::wvalue response; // Prepare success response with subject details.
                    response["name"] = sub_name;
                    response["credits"] = credits;
                    response["quiz_weightage"] = quiz_weightage;
                    response["assignment_weightage"] = assignment_weightage;
                    response["mids_weightage"] = mids_weightage;
                    response["finals_weightage"] = finals_weightage;
                    return crow::response(200, response); // Return success response.
                }
                else {
                    return crow::response(400, "Error creating subject"); // Respond with error if subject creation fails.
                }
            }
            catch (const exception& e) {
                return crow::response(400, "Error creating subject"); // Respond with error if subject creation fails.
            }
        };

        // Handle GET requests for retrieving all subjects.
        if (request.method == "GET"_method) {
            if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin)) {
                return crow::response(401, "You must be logged in to view this data"); // Reject unauthorized requests.
            }

            vector<vector<string>> subjects = getAllSubjects(db); // Fetch all subjects from the database.
            crow::json::wvalue response = crow::json::wvalue::object(); // Prepare JSON response.
            vector<crow::json::wvalue> responseArray;
            crow::json::wvalue json_item;

            for (int i = 0; i < subjects.size(); i++) { // Populate the JSON response with subject details.
                json_item["id"] = subjects[i][0]; // Subject ID.
                json_item["name"] = subjects[i][1]; // Subject name.
                json_item["credits"] = subjects[i][2]; // Credits.
                json_item["quiz_weightage"] = subjects[i][3]; // Quiz weightage.
                json_item["assignment_weightage"] = subjects[i][4]; // Assignment weightage.
                json_item["mids_weightage"] = subjects[i][5]; // Midterm weightage.
                json_item["finals_weightage"] = subjects[i][6]; // Final exam weightage.
                responseArray.push_back(move(json_item));
            }
            response["subjects"] = move(responseArray); // Add the subjects array to the response.
            return crow::response(200, response); // Return response with all subject details.
        }
            });

    CROW_ROUTE(studentSync, "/api/subjects/<int>/") // API endpoint to manage a specific subject. Accessible via GET, PUT, and DELETE methods.
        .methods("GET"_method, "PUT"_method, "DELETE"_method)
        ([db](const crow::request& request, int id) {
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Extract token by removing "Bearer ".
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "You must be logged in to view this data"); // Reject unauthorized requests.
        }

        // Handle GET requests for retrieving a specific subject and its associated tasks.
        if (request.method == "GET"_method) {
            string sqlQuery = "SELECT id, name, credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage FROM subject WHERE id = " + to_string(id) + ";";
            sqlite3_stmt* stmt;
            crow::json::wvalue subject; // Prepare JSON response for the subject.
            sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr);
            sqlite3_step(stmt);

            // Populate the subject details from the database.
            subject["id"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            subject["name"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            subject["credits"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            subject["quiz_weightage"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
            subject["assignment_weightage"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
            subject["mids_weightage"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
            subject["finals_weightage"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));
            sqlite3_finalize(stmt);

            // Add associated tasks (quizzes, assignments, mids, finals) to the response.
            subject["quizes"] = getAllQuizes(db, id);
            subject["assignments"] = getAllAssignments(db, id);
            subject["mids"] = getAllMids(db, id);
            subject["finals"] = getAllFinals(db, id);

            return crow::response(200, subject); // Return subject details with associated tasks.
        }

        // Handle PUT requests for updating a specific subject.
        else if (request.method == "PUT"_method) {
            if (!adminIsAdmin) {
                return crow::response(401, "You are not authorized to perform this action"); // Reject if the user is not an admin.
            }

            auto json_data = crow::json::load(request.body); // Parse JSON data from the request body.
            if (!json_data) {
                return crow::response(400, "Invalid JSON"); // Reject if JSON is invalid or missing.
            }

            string name, credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage;
            try {
                // Extract updated subject details from JSON.
                name = json_data["name"].s();
                credits = json_data["credits"].s();
                quiz_weightage = json_data["quiz_weightage"].s();
                assignment_weightage = json_data["assignment_weightage"].s();
                mids_weightage = json_data["mids_weightage"].s();
                finals_weightage = json_data["finals_weightage"].s();
            }
            catch (const exception& e) {
                return crow::response(400, "Invalid JSON"); // Reject if JSON parsing fails.
            }

            string sql = "UPDATE subject SET name = '" + name + "', credits = " + credits + ", quiz_weightage = " + quiz_weightage +
                ", assignment_weightage = " + assignment_weightage + ", mids_weightage = " + mids_weightage +
                ", finals_weightage = " + finals_weightage + " WHERE id = " + to_string(id) + ";";

            int result = executeSQL(db, sql.c_str()); // Execute the SQL query to update the subject.
            if (result == 0) {
                crow::json::wvalue response; // Prepare success response with updated subject details.
                response["id"] = id;
                response["name"] = name;
                response["credits"] = credits;
                response["quiz_weightage"] = quiz_weightage;
                response["assignment_weightage"] = assignment_weightage;
                response["mids_weightage"] = mids_weightage;
                response["finals_weightage"] = finals_weightage;
                return crow::response(200, response); // Return success response.
            }
            else {
                return crow::response(400, "Error updating subject"); // Respond with error if subject update fails.
            }
        }

        // Handle DELETE requests for deleting a specific subject.
        else if (request.method == "DELETE"_method) {
            if (!adminIsAdmin) {
                return crow::response(401, "You are not authorized to perform this action"); // Reject if the user is not an admin.
            }

            string sql = "DELETE FROM subject WHERE id = " + to_string(id) + ";"; // SQL query to delete the subject.
            int result = executeSQL(db, sql.c_str()); // Execute the SQL query.
            if (result == 0) {
                return crow::response(200, "Subject deleted"); // Respond with success if the subject is deleted.
            }
            else {
                return crow::response(400, "Error deleting subject"); // Respond with error if subject deletion fails.
            }
        }
            });

    CROW_ROUTE(studentSync, "/api/subjects/<int>/add-task/") // API endpoint to add a task (quiz, assignment, mid, final) to a specific subject. Accessible via POST method.
        .methods("POST"_method)
        ([db](const crow::request& request, int id) {
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Extract token by removing "Bearer ".

        // Validate the user's authentication and authorization status.
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "You must be logged in to view this data"); // Reject if the user is not logged in or the token is invalid.
        }
        if (!adminIsAdmin) {
            return crow::response(401, "You are not authorized to perform this action"); // Reject if the user is not an admin.
        }

        auto json_data = crow::json::load(request.body); // Parse JSON data from the request body.
        if (!json_data) {
            return crow::response(400, "Invalid JSON"); // Reject if JSON is invalid or missing.
        }

        // Extract task details from JSON.
        string taskType = json_data["taskType"].s(); // Type of task (e.g., quiz, assignment, mid, final).
        string taskName = json_data["taskName"].s(); // Name of the task.
        int taskMarks = json_data["taskMarks"].i();  // Marks assigned to the task.

        // Construct the SQL query based on the task type.
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
            return crow::response(400, "Invalid task type"); // Reject if the task type is invalid.
        }

        // Execute the SQL query to add the task.
        int result = executeSQL(db, sql.c_str());
        if (result == 0) {
            return crow::response(200, "Task added"); // Respond with success if the task is added.
        }
        else {
            return crow::response(400, "Error adding task"); // Respond with error if task addition fails.
        }
            });

    CROW_ROUTE(studentSync, "/api/subjects/activity/<int>/") // API endpoint to delete a specific task (quiz, assignment, mid, final) from a subject. Accessible via DELETE method.
        .methods("DELETE"_method)
        ([db](const crow::request& request, int id) {
        string adminUsername;
        bool adminIsAdmin;
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Extract token by removing "Bearer ".

        // Validate the user's authentication and authorization status.
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "You must be logged in to view this data"); // Reject unauthorized requests.
        }
        if (!adminIsAdmin) {
            return crow::response(401, "You are not authorized to perform this action"); // Reject if the user is not an admin.
        }

        // Parse the JSON request body to retrieve the task type.
        auto json_data = crow::json::load(request.body);
        if (!json_data) {
            return crow::response(400, "Invalid JSON"); // Reject if JSON is invalid or missing.
        }

        // Extract the task type from the JSON data.
        string taskType = json_data["taskType"].s();
        string sql;

        // Construct the SQL query based on the task type.
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
            return crow::response(400, "Invalid task type"); // Reject if the task type is invalid.
        }

        // Execute the SQL query to delete the task.
        int result = executeSQL(db, sql.c_str());
        if (result == 0) {
            return crow::response(200, "Task deleted"); // Respond with success if the task is deleted.
        }
        else {
            return crow::response(400, "Error deleting task"); // Respond with error if task deletion fails.
        }
            });

    CROW_ROUTE(studentSync, "/api/change-password/") // API endpoint to change the user's password. Accessible via POST method.
        .methods("POST"_method)
        ([db](const crow::request& request) {
        string email, oldPassword, newPassword;

        // Validate the Authorization Token.
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Remove "Bearer ".
        if (auth_header.empty() || !validate_token(auth_header, secretToken)) {
            return crow::response(401, "Authorization token is missing or invalid"); // Reject if the token is missing or invalid.
        }

        // Decode the token to retrieve the user's email and admin status.
        bool isAdmin;
        if (!decodeToken(auth_header, email, isAdmin)) {
            return crow::response(401, "Failed to decode token"); // Reject if token decoding fails.
        }

        // Parse the JSON request body to retrieve the old and new passwords.
        auto json_data = crow::json::load(request.body);
        if (!json_data) {
            return crow::response(400, "Invalid JSON"); // Reject if JSON is invalid or missing.
        }

        try {
            oldPassword = json_data["oldPassword"].s(); // Extract the old password from JSON.
            newPassword = json_data["newPassword"].s(); // Extract the new password from JSON.
        }
        catch (const exception& e) {
            return crow::response(400, "Invalid JSON structure"); // Reject if the JSON structure is invalid.
        }

        // Verify the old password using the `login` function.
        if (!login(db, email, oldPassword)) {
            return crow::response(401, "Incorrect old password"); // Reject if the old password is incorrect.
        }

        // Update the password using the `changePassword` function.
        if (!changePassword(db, email, newPassword)) {
            return crow::response(500, "Failed to update password"); // Respond with an error if the password update fails.
        }

        return crow::response(200, "Password updated successfully"); // Respond with success if the password is updated.
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

    CROW_ROUTE(studentSync, "/api/quiz/") // API endpoint to create a new quiz. Accessible via POST method.
        .methods("POST"_method)
        ([db](const crow::request& request) {
        string adminUsername;
        bool adminIsAdmin;

        // Validate the Authorization Token.
        string auth_header = string(request.get_header_value("Authorization")).replace(0, 7, ""); // Remove "Bearer ".
        if (auth_header.empty() || !validate_token(auth_header, secretToken)) {
            return crow::response(401, "Authorization token is missing or invalid"); // Reject unauthorized requests.
        }

        // Decode the token to retrieve the admin's username and admin status.
        if (!decodeToken(auth_header, adminUsername, adminIsAdmin)) {
            return crow::response(401, "Failed to decode token"); // Reject if token decoding fails.
        }

        // Ensure the user is an admin.
        if (!adminIsAdmin) {
            return crow::response(401, "You are not authorized to perform this action"); // Reject if the user is not an admin.
        }

        // Parse the JSON request body to retrieve the quiz details.
        auto json_data = crow::json::load(request.body);
        if (!json_data) {
            return crow::response(400, "Invalid JSON"); // Reject if JSON is invalid or missing.
        }

        string quizName;
        int subjectId;
        int quizMarks;

        try {
            quizName = json_data["quizName"].s(); // Extract the quiz name.
            subjectId = json_data["subjectId"].i(); // Extract the subject ID to which the quiz belongs.
            quizMarks = json_data["quizMarks"].i(); // Extract the total marks for the quiz.
        }
        catch (const exception& e) {
            return crow::response(400, "Invalid JSON structure"); // Reject if JSON structure is incorrect.
        }

        // SQL query to insert the new quiz into the database.
        string sql = "INSERT INTO quizes (subject_id, quiz_name, quiz_marks) VALUES (" +
            to_string(subjectId) + ", '" + quizName + "', " + to_string(quizMarks) + ");";

        // Execute the SQL query and handle the response.
        try {
            int result = executeSQL(db, sql.c_str());
            if (result == 0) {
                crow::json::wvalue response; // Prepare success response with quiz details.
                response["quizName"] = quizName;
                response["subjectId"] = subjectId;
                response["quizMarks"] = quizMarks;
                return crow::response(200, response); // Return success response.
            }
            else {
                return crow::response(400, "Error creating quiz"); // Respond with error if quiz creation fails.
            }
        }
        catch (const exception& e) {
            return crow::response(500, "Database error: Could not create quiz"); // Respond with error if database operation fails.
        }
            });

    CROW_ROUTE(studentSync, "/api/grouping-sessions/") // API endpoint to manage grouping sessions. Accessible via GET and POST methods.
        .methods("GET"_method, "POST"_method)
        ([](const crow::request& req) {
        auto x = crow::json::load(req.body);

        if (req.method == "GET"_method) { // Handle GET requests to retrieve all grouping sessions.
            vector<vector<string>> sessions = readSessions(); // Fetch all grouping sessions.
            crow::json::wvalue result;
            for (int i = 0; i < sessions.size(); i++) { // Populate response JSON with session details.
                result[i]["id"] = sessions[i][0];
                result[i]["sessionName"] = sessions[i][1];
                result[i]["membersPerGroup"] = sessions[i][2];
                result[i]["status"] = sessions[i][3];
            }
            return crow::response(200, result); // Return the sessions as a JSON response.
        }

        if (req.method == "POST"_method) { // Handle POST requests to create a new grouping session.
            string sessionName = x["sessionName"].s(); // Extract the session name from JSON.
            int membersPerGroup = x["groupMembers"].i(); // Extract the number of members per group.
            cout << sessionName << " " << membersPerGroup << endl;
            addSession(sessionName, membersPerGroup, "Ongoing"); // Add a new session with status "Ongoing".
            return crow::response(200, "Session added successfully");
        }
            });

    CROW_ROUTE(studentSync, "/api/grouping-sessions/<int>/") // API endpoint to manage a specific grouping session. Accessible via GET, DELETE, and PUT methods.
        .methods("GET"_method, "DELETE"_method, "PUT"_method)
        ([](const crow::request& req, int id) {
        auto x = crow::json::load(req.body);
        string username;
        bool adminIsAdmin;

        // Validate the Authorization Token.
        string auth_header = string(req.get_header_value("Authorization")).replace(0, 7, "");
        if (auth_header.empty() || !validate_token(auth_header, secretToken) || !decodeToken(auth_header, username, adminIsAdmin)) {
            return crow::response(401, "You must be logged in to view this data");
        }

        if (req.method == "GET"_method) { // Handle GET requests to retrieve details of a specific session.
            vector<vector<string>> sessions = readSessions(); // Fetch all sessions.
            crow::json::wvalue result;
            for (int i = 0; i < sessions.size(); i++) { // Find the session with the matching ID.
                if (stoi(sessions[i][0]) == id) {
                    result["id"] = sessions[i][0];
                    result["sessionName"] = sessions[i][1];
                    result["membersPerGroup"] = sessions[i][2];
                    result["status"] = sessions[i][3];
                    result["preferenceGiven"] = preferenceAlreadyGiven("GroupFormer/" + sessions[i][1] + "_" + sessions[i][0] + ".csv", username); // Check if user has given preferences.
                }
            }
            return crow::response(200, result); // Return session details as a JSON response.
        }

        if (req.method == "DELETE"_method) { // Handle DELETE requests to delete a session.
            deleteSession(id); // Delete the session by ID.
            return crow::response(200, "Session deleted successfully");
        }

        if (req.method == "PUT"_method) { // Handle PUT requests to update session status.
            string status = x["status"].s(); // Extract the new status from JSON.
            updateSessionStatus(id, status); // Update the session's status.
            return crow::response(200, "Session updated successfully");
        }
            });

    CROW_ROUTE(studentSync, "/api/group-preferences/") // API endpoint to save group preferences for a session. Accessible via POST method.
        .methods("POST"_method)
        ([](const crow::request& req) {
        auto x = crow::json::load(req.body);

        if (req.method == "POST"_method) { // Handle POST requests to submit preferences.
            string sessionName = x["sessionName"].s(); // Extract session name.
            int sessionId = x["sessionId"].i(); // Extract session ID.
            auto preferences = x["preferences"]; // Extract user preferences.
            string userEmail = x["email"].s(); // Extract user email.
            vector<string> preferencesArray;
            for (int i = 0; i < preferences.size(); i++) { // Parse preferences into a vector.
                preferencesArray.push_back(preferences[i].s());
            }
            string filename = "GroupFormer/" + sessionName + "_" + to_string(sessionId) + ".csv"; // Filename for storing preferences.
            TakePreferences(filename, userEmail, preferencesArray); // Save preferences to the file.
            return crow::response(200, "Preferences added successfully");
        }
            });

    CROW_ROUTE(studentSync, "/api/form-groups/") // API endpoint to form groups based on user preferences. Accessible via POST method.
        .methods("POST"_method)
        ([db](const crow::request& req) {
        auto x = crow::json::load(req.body);

        if (req.method == "POST"_method) { // Handle POST requests to form groups.
            string sessionName = x["sessionName"].s(); // Extract session name.
            int sessionId = x["sessionId"].i(); // Extract session ID.
            string filename1 = "GroupFormer/" + sessionName + "_" + to_string(sessionId) + ".csv"; // Input preferences file.
            string filename2 = "GroupFormer/" + sessionName + "_" + to_string(sessionId) + "result.csv"; // Output groups file.
            int membersPerGroup = x["membersPerGroup"].i(); // Extract number of members per group.
            vector<string> emails;

            // Fetch all student emails from the database.
            string sql = "SELECT email, name FROM users";
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                string email = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
                if (email.find(".bese24seecs") != string::npos) { // Filter student emails.
                    emails.push_back(email);
                }
            }
            sqlite3_finalize(stmt);

            FormGroups(filename1, filename2, membersPerGroup, emails); // Form groups based on preferences.
            updateSessionStatus(sessionId, "Completed"); // Update session status to "Completed".
            return crow::response(200, "Group formed successfully");
        }
            });

    CROW_ROUTE(studentSync, "/api/grouping-sessions/groups/") // API endpoint to retrieve formed groups for a session. Accessible via POST method.
        .methods("POST"_method)
        ([](const crow::request& req) {
        if (req.method == "POST"_method) { // Handle POST requests to fetch groups.
            vector<vector<string>> groups;
            auto x = crow::json::load(req.body);
            string sessionName = x["sessionName"].s(); // Extract session name.
            int id = x["id"].i(); // Extract session ID.
            string filename = "GroupFormer/" + sessionName + "_" + to_string(id) + "result.csv"; // File containing groups.
            ifstream file(filename); // Open the file for reading.
            string line, temp;
            vector<string> row;

            // Read groups from the file line by line.
            while (getline(file, line)) {
                for (char c : line) {
                    if (c == ',') {
                        row.push_back(temp);
                        temp = "";
                    }
                    else {
                        temp += c;
                    }
                }
                groups.push_back(row); // Add group to the result.
                row.clear();
            }

            // Prepare the response JSON.
            crow::json::wvalue result;
            for (int i = 0; i < groups.size(); i++) {
                result[i] = groups[i];
            }
            return crow::response(200, result); // Return groups as a JSON response.
        }
            });

    studentSync.port(2028).multithreaded().run();
}

// Function to store or update student marks in a CSV file for a specific assessment.
bool studentMarksToFile(string userEmail, string assesmentType, string obtainedMarks, string subjectId, string subName, string assesmentId) {
    fstream file;
    string filePath = "student_marks/" + subName + "_" + subjectId + ".csv"; // File path for the subject's marks file.
    vector<string> data; // To store lines read from the file.
    string line;

    // Open the file for reading (and appending if it doesn't exist).
    file.open(filePath, ios::in | ios::app);
    if (!file.is_open()) {
        cout << "Error opening file for reading." << endl;
        return false; // Return false if the file cannot be opened.
    }

    // Read all lines from the file into a vector for processing.
    while (getline(file, line)) {
        data.push_back(line);
    }
    file.close(); // Close the file after reading.

    // Open the file for writing to update or append data.
    file.open(filePath, ios::out);
    bool done = false; // Flag to check if the marks were updated.

    for (int i = 0; i < data.size(); i++) {
        // Check if the line matches the user's email, assessment type, and ID.
        if (data[i].find(userEmail) != string::npos && data[i].find(assesmentType) != string::npos && data[i].find(assesmentId) != string::npos) {
            // Update the line with new marks.
            data[i] = userEmail + "," + assesmentType + "," + assesmentId + "," + obtainedMarks;
            done = true; // Marks were updated.
        }
        file << data[i] << endl; // Write the line back to the file.
    }

    // Append new marks if they were not updated in existing data.
    if (!done) {
        file << userEmail + "," + assesmentType + "," + assesmentId + "," + obtainedMarks << endl;
    }

    file.close(); // Close the file after writing.
    return true; // Return true if the operation was successful.
}

// Function to retrieve student marks for a specific assessment from a CSV file.
int studentMarksFromFile(string userEmail, string assesmentType, int subjectId, string subName, int assesmentId) {
    fstream file;
    string filePath = "student_marks/" + subName + "_" + to_string(subjectId) + ".csv"; // File path for the subject's marks file.

    file.open(filePath, ios::in); // Open the file for reading.
    if (!file.is_open()) {
        return 0; // Return 0 if the file cannot be opened.
    }

    string line;
    // Read the file line by line.
    while (getline(file, line)) {
        // Check if the line matches the user's email, assessment type, and ID.
        if (line.find(userEmail) != string::npos && line.find(assesmentType) != string::npos && line.find(to_string(assesmentId)) != string::npos) {
            // Extract the marks from the last field in the line and return as integer.
            string marks = line.substr(line.find_last_of(",") + 1);
            return stoi(marks);
        }
    }
    return 0; // Return 0 if the marks were not found.
}

// Validates if a given date string matches the ISO 8601 format.
bool matchesIso8601(const string& date) {
    // Regular expression for validating ISO 8601 format (e.g., YYYY-MM-DDTHH:MM:SSZ).
    const regex iso8601Regex(R"(^\d{4}-[01]\d-[0-3]\dT[0-2]\d:[0-5]\d(:[0-5]\d(\.\d+)?([+-][0-2]\d:[0-5]\d|Z)?)?$)");

    return regex_match(date, iso8601Regex); // Return true if the string matches the regex, otherwise false.
}

// Converts a given string to lowercase.
string toLowerCase(const string& input) {
    string lowerCaseInput = input; // Create a copy of the input string.
    for (size_t i = 0; i < lowerCaseInput.length(); i++) {
        lowerCaseInput[i] = tolower(lowerCaseInput[i]); // Convert each character to lowercase.
    }
    return lowerCaseInput; // Return the lowercase string.
}

// Executes the provided SQL query on the database.
int executeSQL(sqlite3* db, const char* sql) {
    char* errorMessage; // Pointer to store any error messages returned by SQLite.
    int exit = sqlite3_exec(db, sql, nullptr, 0, &errorMessage); // Execute the SQL query.

    if (exit != SQLITE_OK) { // Check if the execution failed.
        cout << "Error executing SQL: " << errorMessage << endl; // Log the error message.
        sqlite3_free(errorMessage); // Free the allocated memory for the error message.
        return 1; // Return 1 to indicate failure.
    }
    return 0; // Return 0 to indicate success.
}

// Generates a JWT token for a user.
string generate_token(sqlite3* db, string username, string password, string secretToken) {
    bool isAdmin; // Flag to indicate if the user has admin privileges.
    string name;  // Variable to store the user's name.

    getUserData(db, username, password, name, isAdmin); // Retrieve user data from the database.
    return jwt::create()
        .set_issuer("StudentSync") // Set the issuer of the token.
        .set_payload_claim("username", jwt::claim(username)) // Add the username as a payload claim.
        .set_payload_claim("role", jwt::claim(string(isAdmin ? "admin" : "user"))) // Add the user's role (admin or user) as a claim.
        .sign(jwt::algorithm::hs256(secretToken)); // Sign the token using HMAC-SHA256 with the secret token.
}

// Validates a JWT token against the secret token.
bool validate_token(string token, string secretToken) {
    try {
        auto decoded_token = jwt::decode(token); // Decode the provided JWT token.
        // Create a verifier that checks the algorithm and issuer.
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256(secretToken)) // Allow only HMAC-SHA256 for verification.
            .with_issuer("StudentSync"); // Ensure the token's issuer matches "StudentSync".
        verifier.verify(decoded_token); // Verify the decoded token's signature and claims.
        return true; // Token is valid.
    }
    catch (const exception& e) {
        return false; // Return false if validation fails or an exception occurs.
    }
}

// Decodes a JWT token to extract the username and role information.
bool decodeToken(string token, string& username, bool& isAdmin) {
    try {
        auto decoded_token = jwt::decode(token); // Decode the provided JWT token.

        // Extract the "username" claim and store it in the username variable.
        username = decoded_token.get_payload_claim("username").as_string();

        // Extract the "role" claim and determine if the user is an admin.
        isAdmin = decoded_token.get_payload_claim("role").as_string() == "admin";

        return true; // Token decoding and data extraction were successful.
    }
    catch (const exception& e) {
        return false; // Return false if decoding fails or an exception occurs.
    }
}
// Ensures a default admin user exists in the database.
void defaultAdminUser(sqlite3* db) {
    if (checkUserExxisits(db, "admin")) { // Check if the admin user already exists.
        return; // Exit if the admin user exists.
    }
    createUser(db, "admin", "admin", "admin", true); // Create a default admin user with username "admin" and password "admin".
}

// Verifies user login credentials.
bool login(sqlite3* db, string email, string password) {
    string sql = "SELECT email FROM users WHERE email = '" + email + "' AND password = '" + password + "';"; // SQL query to verify credentials.
    sqlite3_stmt* stmt;

    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr); // Prepare the SQL statement.
    int result = sqlite3_step(stmt); // Execute the query and check if a row exists.
    sqlite3_finalize(stmt); // Finalize the statement to free resources.

    return result == SQLITE_ROW; // Return true if a matching user is found, otherwise false.
}

// Retrieves user details from the database.
void getUserData(sqlite3* db, string email, string password, string& name, bool& isAdmin) {
    email = toLowerCase(email); // Convert the email to lowercase for uniformity.
    string sql = "SELECT name, isAdmin FROM users WHERE email = '" + email + "' AND password = '" + password + "';"; // SQL query to fetch user data.
    sqlite3_stmt* stmt;

    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr); // Prepare the SQL statement.
    sqlite3_step(stmt); // Execute the query.

    name = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))); // Extract the user's name from the result.
    isAdmin = sqlite3_column_int(stmt, 1); // Extract the user's admin status (1 for true, 0 for false).

    sqlite3_finalize(stmt); // Finalize the statement to free resources.
}

// Checks if a user with the given email exists in the database.
bool checkUserExxisits(sqlite3* db, string email) {
    email = toLowerCase(email); // Convert the email to lowercase for uniformity.
    string sql = "SELECT email FROM users WHERE email = '" + email + "';"; // SQL query to check for user existence.
    sqlite3_stmt* stmt;

    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr); // Prepare the SQL statement.
    int result = sqlite3_step(stmt); // Execute the query and check if a row exists.
    sqlite3_finalize(stmt); // Finalize the statement to free resources.

    return result == SQLITE_ROW; // Return true if a matching email is found, otherwise false.
}

// Creates a new user in the database.
bool createUser(sqlite3* db, string email, string name, string password, bool isAdmin) {
    if (checkUserExxisits(db, email)) { // Check if the user already exists.
        cout << "User already exists" << endl; // Log a message if the user exists.
        return false; // Return false if the user cannot be created.
    }

    string newemail = toLowerCase(email); // Convert the email to lowercase for uniformity.
    string sql = "INSERT INTO users (email, name, password, isAdmin) VALUES ('" + newemail + "', '" + name + "', '" + password + "', " + (isAdmin ? "1" : "0") + ");";

    executeSQL(db, sql.c_str()); // Execute the SQL query to insert the new user.
    return true; // Return true if the user is created successfully.
}

// Deletes a user from the database.
bool deleteUser(sqlite3* db, string email) {
    if (!checkUserExxisits(db, email)) { // Check if the user does not exist.
        cout << "User does not exist" << endl; // Log a message if the user does not exist.
        return false; // Return false if the user cannot be deleted.
    }

    string sql = "DELETE FROM users WHERE email = '" + email + "';"; // SQL query to delete the user.
    executeSQL(db, sql.c_str()); // Execute the SQL query to remove the user.
    return true; // Return true if the user is deleted successfully.
}

// Changes the password of an existing user.
bool changePassword(sqlite3* db, string email, string newPassword) {
    if (!checkUserExxisits(db, email)) { // Check if the user does not exist.
        cout << "User does not exist" << endl; // Log a message if the user does not exist.
        return false; // Return false if the password cannot be changed.
    }

    string sql = "UPDATE users SET password = '" + newPassword + "' WHERE email = '" + email + "';"; // SQL query to update the password.
    executeSQL(db, sql.c_str()); // Execute the SQL query to update the password.
    return true; // Return true if the password is changed successfully.
}

// Creates a new event in the database.
void createEvent(sqlite3* db, string title, string description, string schedule_at) {
    string sql = "INSERT INTO events (title, description, schedule_at) VALUES ('" + title + "', '" + description + "', '" + schedule_at + "');";

    sqlite3_stmt* stmt; // Prepare the SQLite statement.
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr); // Prepare the SQL query.

    if (sqlite3_step(stmt) != SQLITE_DONE) { // Check if the execution failed.
        throw(exception(sqlite3_errmsg(db))); // Throw an exception with the error message.
    }

    sqlite3_finalize(stmt); // Finalize the statement to free resources.
}

// Deletes an event from the database.
void deleteEvent(sqlite3* db, int id) {
    string sql = "DELETE FROM events WHERE id = " + to_string(id) + ";"; // SQL query to delete the event by ID.

    sqlite3_stmt* stmt; // Prepare the SQLite statement.
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr); // Prepare the SQL query.

    if (sqlite3_step(stmt) != SQLITE_DONE) { // Check if the execution failed.
        throw(exception("Error deleting event")); // Throw an exception with an error message if the event could not be deleted.
    }

    sqlite3_finalize(stmt); // Finalize the statement to free resources.
}

// Retrieves all events from the database.
vector<vector<string>> getAllEvents(sqlite3* db) {
    vector<vector<string>> events; // Vector to store all events.
    string sqlQuery = "SELECT id, title, description, schedule_at FROM events;"; // SQL query to fetch all events.
    sqlite3_stmt* stmt; // SQLite statement handle.
    vector<string> tempEvent; // Temporary vector to store individual event details.

    sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr); // Prepare the SQL statement.
    while (sqlite3_step(stmt) == SQLITE_ROW) { // Iterate through the result rows.
        tempEvent.clear(); // Clear the temporary vector for the new event.
        tempEvent.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)))); // Event ID.
        tempEvent.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)))); // Event title.
        tempEvent.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)))); // Event description.
        tempEvent.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)))); // Event schedule.
        events.push_back(tempEvent); // Add the event details to the result vector.
    }

    sqlite3_finalize(stmt); // Finalize the statement to free resources.
    return events; // Return the list of events.
}

// Retrieves all subjects from the database.
vector<vector<string>> getAllSubjects(sqlite3* db) {
    vector<vector<string>> subjects; // Vector to store all subjects.
    string sqlQuery = "SELECT id, name, credits, quiz_weightage, assignment_weightage, mids_weightage, finals_weightage FROM subject;"; // SQL query to fetch all subjects.
    sqlite3_stmt* stmt; // SQLite statement handle.
    vector<string> tempSubject; // Temporary vector to store individual subject details.

    sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr); // Prepare the SQL statement.
    while (sqlite3_step(stmt) == SQLITE_ROW) { // Iterate through the result rows.
        tempSubject.clear(); // Clear the temporary vector for the new subject.
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)))); // Subject ID.
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)))); // Subject name.
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)))); // Subject credits.
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)))); // Quiz weightage.
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)))); // Assignment weightage.
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)))); // Mids weightage.
        tempSubject.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)))); // Finals weightage.
        subjects.push_back(tempSubject); // Add the subject details to the result vector.
    }

    sqlite3_finalize(stmt); // Finalize the statement to free resources.
    return subjects; // Return the list of subjects.
}

// Retrieves all quizzes for a specific subject from the database.
vector<vector<string>> getAllQuizes(sqlite3* db, int subject_id) {
    vector<vector<string>> quizes; // Vector to store all quizzes.
    string sqlQuery = "SELECT id, quiz_name, quiz_marks FROM quizes WHERE subject_id = " + to_string(subject_id) + ";"; // SQL query to fetch quizzes.
    sqlite3_stmt* stmt; // SQLite statement handle.
    vector<string> tempQuiz; // Temporary vector to store individual quiz details.

    sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr); // Prepare the SQL statement.
    while (sqlite3_step(stmt) == SQLITE_ROW) { // Iterate through the result rows.
        tempQuiz.clear(); // Clear the temporary vector for the new quiz.
        tempQuiz.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)))); // Quiz ID.
        tempQuiz.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)))); // Quiz name.
        tempQuiz.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)))); // Quiz marks.
        quizes.push_back(tempQuiz); // Add the quiz details to the result vector.
    }

    sqlite3_finalize(stmt); // Finalize the statement to free resources.
    return quizes; // Return the list of quizzes.
}

// Retrieves all assignments for a specific subject from the database.
vector<vector<string>> getAllAssignments(sqlite3* db, int subject_id) {
    vector<vector<string>> assignments; // Vector to store all assignments.
    string sqlQuery = "SELECT id, assignment_name, assignment_marks FROM assignments WHERE subject_id = " + to_string(subject_id) + ";"; // SQL query to fetch assignments.
    sqlite3_stmt* stmt; // SQLite statement handle.
    vector<string> tempAssignment; // Temporary vector to store individual assignment details.

    sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr); // Prepare the SQL statement.
    while (sqlite3_step(stmt) == SQLITE_ROW) { // Iterate through the result rows.
        tempAssignment.clear(); // Clear the temporary vector for the new assignment.
        tempAssignment.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)))); // Assignment ID.
        tempAssignment.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)))); // Assignment name.
        tempAssignment.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)))); // Assignment marks.
        assignments.push_back(tempAssignment); // Add the assignment details to the result vector.
    }

    sqlite3_finalize(stmt); // Finalize the statement to free resources.
    return assignments; // Return the list of assignments.
}

// Retrieves all mids for a specific subject from the database.
vector<vector<string>> getAllMids(sqlite3* db, int subject_id) {
    vector<vector<string>> mids; // Vector to store all mids.
    string sqlQuery = "SELECT id, mid_name, mid_marks FROM mids WHERE subject_id = " + to_string(subject_id) + ";"; // SQL query to fetch mids.
    sqlite3_stmt* stmt; // SQLite statement handle.
    vector<string> tempMid; // Temporary vector to store individual mid details.

    sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr); // Prepare the SQL statement.
    while (sqlite3_step(stmt) == SQLITE_ROW) { // Iterate through the result rows.
        tempMid.clear(); // Clear the temporary vector for the new mid.
        tempMid.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)))); // Mid ID.
        tempMid.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)))); // Mid name.
        tempMid.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)))); // Mid marks.
        mids.push_back(tempMid); // Add the mid details to the result vector.
    }

    sqlite3_finalize(stmt); // Finalize the statement to free resources.
    return mids; // Return the list of mids.
}

// Retrieves all finals for a specific subject from the database.
vector<vector<string>> getAllFinals(sqlite3* db, int subject_id) {
    vector<vector<string>> finals; // Vector to store all finals.
    string sqlQuery = "SELECT id, final_name, final_marks FROM finals WHERE subject_id = " + to_string(subject_id) + ";"; // SQL query to fetch finals.
    sqlite3_stmt* stmt; // SQLite statement handle.
    vector<string> tempFinal; // Temporary vector to store individual final details.

    sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr); // Prepare the SQL statement.
    while (sqlite3_step(stmt) == SQLITE_ROW) { // Iterate through the result rows.
        tempFinal.clear(); // Clear the temporary vector for the new final.
        tempFinal.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)))); // Final ID.
        tempFinal.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)))); // Final name.
        tempFinal.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)))); // Final marks.
        finals.push_back(tempFinal); // Add the final details to the result vector.
    }

    sqlite3_finalize(stmt); // Finalize the statement to free resources.
    return finals; // Return the list of finals.
}

// Reads all grouping sessions from a CSV file.
vector<vector<string>> readSessions() {
    fstream file; // File stream to read the CSV file.
    string filePath = "GroupFormer/sessions.csv"; // Path to the sessions CSV file.
    vector<vector<string>> data; // Vector to store the session data.
    vector<string> tempData; // Temporary vector for storing one session's data.

    file.open(filePath, ios::in); // Open the file for reading.
    string tempLine, temp; // Variables to store each line and temporary data.

    // Read the file line by line.
    while (getline(file, tempLine)) {
        temp = ""; // Clear temporary data.
        for (int i = 0; i < tempLine.length(); i++) { // Process each character in the line.
            if (tempLine[i] == ',') { // Split data by commas.
                tempData.push_back(temp); // Add the data to the session.
                temp = ""; // Reset for the next data field.
            }
            else {
                temp += tempLine[i]; // Append the character to the current data field.
            }
        }
        tempData.push_back(temp); // Add the last field.
        data.push_back(tempData); // Add the session to the result vector.
        tempData.clear(); // Clear the temporary vector for the next session.
    }

    file.close(); // Close the file after reading.
    return data; // Return the list of sessions.
}

// Adds a new session to the "sessions.csv" file.
void addSession(string sessionName, int groupMembers, string status) {
    fstream file;
    string filePath = "GroupFormer/sessions.csv"; // Path to the sessions CSV file.
    vector<string> data; // Vector to store existing sessions.

    file.open(filePath, ios::in | ios::out); // Open the file for reading and writing.
    if (!file) { // If the file does not exist.
        file.open(filePath, ios::out); // Create the file.
        file.close();
        file.open(filePath, ios::in | ios::out); // Reopen the file.
    }

    string tempLine;
    while (getline(file, tempLine)) { // Read existing sessions.
        data.push_back(tempLine);
    }

    file.clear(); // Clear flags after reading.
    file.seekg(0, ios::end); // Move the file pointer to the end.
    file << data.size() + 1 << "," << sessionName << "," << groupMembers << "," << status << endl; // Add the new session.
    file.close(); // Close the file.
}

// Deletes a session from the "sessions.csv" file by its ID.
void deleteSession(int id) {
    fstream file;
    string filePath = "GroupFormer/sessions.csv"; // Path to the sessions CSV file.
    vector<string> data; // Vector to store remaining sessions.

    file.open(filePath, ios::in); // Open the file for reading.
    string tempLine;

    while (getline(file, tempLine)) { // Read each session.
        int commaPos = tempLine.find(','); // Find the first comma to extract the session ID.
        if (commaPos != string::npos) {
            string sessionId = tempLine.substr(0, commaPos); // Extract the session ID.
            if (sessionId != to_string(id)) { // Exclude the session with the matching ID.
                data.push_back(tempLine);
            }
        }
        else {
            data.push_back(tempLine); // Add the line if it doesn't have a valid format.
        }
    }

    file.close(); // Close the file after reading.

    file.open(filePath, ios::out | ios::trunc); // Open the file for writing and truncate it.
    for (const auto& line : data) { // Write the remaining sessions back to the file.
        file << line << endl;
    }
    file.close(); // Close the file.
}

// Updates the status of a session in the "sessions.csv" file by its ID.
void updateSessionStatus(int id, string status) {
    fstream file;
    string filePath = "GroupFormer/sessions.csv"; // Path to the sessions CSV file.
    vector<string> data; // Vector to store sessions.

    file.open(filePath, ios::in); // Open the file for reading.
    string tempLine;
    while (getline(file, tempLine)) { // Read all sessions into the vector.
        data.push_back(tempLine);
    }
    file.close(); // Close the file after reading.

    file.open(filePath, ios::out | ios::trunc); // Open the file for writing and truncate it.
    for (int i = 0; i < data.size(); i++) {
        if (data[i].find(to_string(id)) != string::npos) { // Locate the session by ID.
            vector<string> tempData; // Split the session line into components.
            string temp = "";
            for (int j = 0; j < data[i].length(); j++) {
                if (data[i][j] == ',') {
                    tempData.push_back(temp);
                    temp = "";
                }
                else {
                    temp += data[i][j];
                }
            }
            tempData.push_back(temp); // Add the last field.
            data[i] = tempData[0] + "," + tempData[1] + "," + tempData[2] + "," + status; // Update the status.
        }
        file << data[i] << endl; // Write the session back to the file.
    }
    file.close(); // Close the file.
}

// Reads questions from a CSV file and returns them as a vector of `Question`.
vector<Question> readCSV(const string& filename) {
    ifstream file(filename); // Open the CSV file for reading.
    string line;
    vector<Question> Questions; // Vector to store questions.

    while (getline(file, line)) { // Read each line from the file.
        int commaPos1 = line.find(','); // Find the first comma.
        int commaPos2 = line.rfind(','); // Find the last comma.
        if (commaPos1 != string::npos && commaPos2 != string::npos && commaPos1 != commaPos2) {
            Question Question; // Create a new Question object.
            Question.id = stoi(line.substr(0, commaPos1)); // Extract the ID.
            Question.question = line.substr(commaPos1 + 1, commaPos2 - commaPos1 - 1); // Extract the question.
            Question.answer = line.substr(commaPos2 + 1); // Extract the answer.
            Questions.push_back(Question); // Add the question to the vector.
        }
    }
    return Questions; // Return the list of questions.
}

// Writes a vector of `Question` to a CSV file.
void writeCSV(const string& filename, const vector<Question>& Questions) {
    ofstream file(filename); // Open the CSV file for writing.
    for (const auto& Question : Questions) { // Write each question to the file.
        file << Question.id << "," << Question.question << "," << Question.answer << "\n";
    }
}

// Appends a single question to a CSV file.
void appendCSV(const string& filename, const Question& Question) {
    ofstream file(filename, ios::app); // Open the CSV file in append mode.
    file << Question.id << "," << Question.question << "," << Question.answer << "\n"; // Append the question.
}

// Adds a new question to a subject and updates the corresponding CSV file.
void addQuestion(unordered_map<string, vector<Question>>& subjectMap, const string& subject, int id, const string& question, const string& answer) {
    Question newQuestion = { id, question, answer }; // Create a new Question object.
    subjectMap[subject].push_back(newQuestion); // Add the question to the subject's list.
    appendCSV("quizes/" + subject + ".csv", newQuestion); // Append the question to the CSV file.
}

// Deletes a question from a vector and reassigns IDs to ensure sequential numbering.
void deleteRow(vector<Question>& Questions, int delete_index) {
    if (delete_index >= 0 && delete_index < Questions.size()) { // Check if the index is valid.
        Questions.erase(Questions.begin() + delete_index); // Remove the question.
        for (int i = delete_index; i < Questions.size(); ++i) { // Reassign sequential IDs.
            Questions[i].id = i + 1;
        }
    }
}

// Displays all questions as a formatted string.
string displayData(const vector<Question>& Questions) {
    string result = "ID,Question,Answer\n"; // Add the header.
    for (const auto& Question : Questions) { // Add each question to the result string.
        result += to_string(Question.id) + "," + Question.question + "," + Question.answer + "\n";
    }
    return result; // Return the formatted string.
}

// Reads the entire content of a file and returns it as a string.
string readFile(const string& filename) {
    ifstream file(filename); // Open the file for reading.
    if (!file.is_open()) { // Check if the file could not be opened.
        cerr << "Error: Cannot open file " << filename << endl;
        return ""; // Return an empty string if the file cannot be opened.
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>()); // Read the file content.
    return content; // Return the content.
}

// Appends user preferences to a CSV file.
void TakePreferences(const string& filename, const string& mail, const vector<string>& preferences) {
    ofstream Outfile(filename, ios::app); // Open the file in append mode.
    if (!Outfile.is_open()) { // Check if the file could not be opened.
        cerr << "Error: Could not open the file." << endl;
        return;
    }

    Outfile << mail << ","; // Add the email.
    for (const auto& pref : preferences) { // Add each preference.
        Outfile << pref << ",";
    }
    Outfile << "\n"; // Add a newline at the end.

    Outfile.close(); // Close the file.
}

// Checks if a user's preferences already exist in the file.
bool preferenceAlreadyGiven(const string& filename, const string& mail) {
    ifstream Infile(filename); // Open the file for reading.
    string line;
    while (getline(Infile, line)) { // Read each line.
        line = toLowerCase(line); // Convert the line to lowercase.
        line = line.substr(0, line.find(',')); // Extract the email from the line.
        if (line.find(mail) != string::npos) { // Check if the email matches.
            Infile.close(); // Close the file.
            return true; // Return true if preferences already exist.
        }
    }
    Infile.close(); // Close the file if no match is found.
    return false; // Return false if preferences are not found.
}

// Helper function Checks if two users have a mutual preference for each other.
// The function verifies whether `email1` is listed as a preference in `email2`'s preference list.
bool checkMutualPreference(string email1, string email2, unordered_map<string, vector<string>>& preferences) {
    // Iterate through the preferences of `email2`.
    for (int i = 0; i < preferences[email2].size(); i++) {
        // Check if `email1` is in `email2`'s preference list.
        if (preferences[email2][i] == email1) {
            return true; // Return true if mutual preference exists.
        }
    }
    return false; // Return false if no mutual preference is found.
}

void FormGroups(const string filename1, const string filename2, int groupSize, vector<string> emails) {
    srand(time(0)); // Seed the random number generator for randomness.
    vector<vector<string>> groups; // Vector to store the final groups.
    unordered_map<string, vector<string>> preferences; // Map to store user preferences from the file.
    unordered_set<string> grouped; // Set to track already grouped users.
    fstream file;
    file.open(filename1, ios::in); // Open the preferences file for reading.
    string line, tempEmail;

    // Reading preferences from file
    while (getline(file, line)) { // Read each line of preferences.
        string temp;
        vector<string> row;
        tempEmail = "";
        for (int i = 0; i < line.size(); i++) { // Parse the line character by character.
            if (line[i] == ',') {
                if (tempEmail == "") {
                    tempEmail = temp; // First entry is the user's email.
                }
                else {
                    row.push_back(temp); // Remaining entries are preferences.
                }
                temp = "";
            }
            else {
                temp += line[i]; // Build the current field.
            }
        }
        if (temp.size() > 0) {
            row.push_back(temp); // Add the last field.
        }
        preferences[tempEmail] = row; // Map email to its preferences.
    }
    file.close(); // Close the preferences file.

    bool mutual;
    vector<string> tempGroup;

    // Initial grouping based on preferences
    for (auto pair : preferences) { // Iterate over each user's preferences.
        if (grouped.count(pair.first) == 1) { // Skip if the user is already grouped.
            continue;
        }
        for (int i = 0; i < pair.second.size(); i++) { // Check each preference of the user.
            if (grouped.count(pair.second[i]) == 1) { // Skip if the preferred user is already grouped.
                continue;
            }
            else {
                mutual = checkMutualPreference(pair.first, pair.second[i], preferences); // Check mutual preference.
                if (mutual) {
                    tempGroup.clear();
                    tempGroup.push_back(pair.first); // Add the user to the group.
                    tempGroup.push_back(pair.second[i]); // Add the preferred user to the group.
                    groups.push_back(tempGroup); // Add the group to the list.
                    grouped.insert(pair.first); // Mark both users as grouped.
                    grouped.insert(pair.second[i]);
                    break; // Stop once a group is formed.
                }
            }
        }
    }

    // Fill groups to required size
    for (auto& group : groups) { // Iterate over each group.
        if (group.size() >= groupSize) { // Skip if the group is already full.
            continue;
        }
        for (int i = 0; i < group.size(); i++) { // Iterate over group members.
            auto tempPreferences = preferences[group[i]]; // Get preferences of the group member.
            for (auto preference : tempPreferences) { // Check each preference.
                if (group.size() >= groupSize) { // Stop if the group is full.
                    break;
                }
                if (grouped.count(preference)) { // Skip if the preferred user is already grouped.
                    continue;
                }
                mutual = checkMutualPreference(group[i], preference, preferences); // Check mutual preference.
                if (mutual) {
                    group.push_back(preference); // Add the user to the group.
                    grouped.insert(preference);
                }
            }
            if (group.size() >= groupSize) { // Stop if the group is full.
                break;
            }
        }
    }

    for (auto& group : groups) { // Iterate over each group.
        unordered_map<string, int> emailCount; // Map to count email occurrences.
        if (group.size() >= groupSize) { // Skip if the group is already full.
            continue;
        }
        for (int i = 0; i < group.size(); i++) { // Iterate over group members.
            if (group.size() >= groupSize) { // Stop if the group is full.
                break;
            }
            auto tempPreferences = preferences[group[i]]; // Get preferences of the group member.
            for (auto preference : tempPreferences) { // Check each preference.
                if (group.size() >= groupSize) { // Stop if the group is full.
                    break;
                }
                if (grouped.count(preference) >= 1) { // Skip if the preferred user is already grouped.
                    continue;
                }
                if (emailCount.count(preference) == 0) {
                    emailCount[preference] = 1; // Initialize the count.
                }
                else {
                    emailCount[preference]++; // Increment the count.
                }
            }
        }

        vector<pair<int, string>> sortedEmails; // Vector to store sorted emails.
        for (const auto& entry : emailCount) {
            sortedEmails.push_back({ entry.second, entry.first }); // Store email and its count.
        }

        // Sort the emails based on their frequency in descending order
        sort(sortedEmails.rbegin(), sortedEmails.rend());

        // Add the most popular emails to the group until it reaches the required size
        for (auto& email : sortedEmails) {
            if (group.size() >= groupSize) {
                break;
            }
            if (grouped.count(email.second) == 0) {
                group.push_back(email.second);
                grouped.insert(email.second);
            }
        }
    }

    int requiredGroups = ceil((double(emails.size()) / double(groupSize))) - groups.size(); // Calculate required groups.
    vector<string> remainingEmails; // Store emails that haven't been grouped.

    // Collect remaining emails that haven't been grouped
    for (auto email : emails) {
        if (grouped.count(email) == 0) {
            remainingEmails.push_back(email); // Add ungrouped emails.
        }
    }

    int randEmail;
    for (int i = 0; i < requiredGroups; i++) { // Create random groups for ungrouped emails.
        if (remainingEmails.empty()) break;
        randEmail = (rand() % remainingEmails.size());
        tempGroup.clear();
        tempGroup.push_back(remainingEmails[randEmail]); // Start a new group with a random email.
        grouped.insert(remainingEmails[randEmail]);
        remainingEmails.erase(remainingEmails.begin() + randEmail);
        groups.push_back(tempGroup);
    }

    // Fill groups to required size
    for (auto& group : groups) { // Iterate over each group.
        if (group.size() >= groupSize) { // Skip if the group is already full.
            continue;
        }
        for (int i = 0; i < group.size(); i++) { // Iterate over group members.
            auto tempPreferences = preferences[group[i]]; // Get preferences of the group member.
            for (auto preference : tempPreferences) { // Check each preference.
                if (group.size() >= groupSize) { // Stop if the group is full.
                    break;
                }
                if (grouped.count(preference) >= 1) { // Skip if the preferred user is already grouped.
                    continue;
                }
                mutual = checkMutualPreference(group[i], preference, preferences); // Check mutual preference.
                if (mutual) {
                    group.push_back(preference);
                    grouped.insert(preference);
                }
            }
            if (group.size() >= groupSize) { // Stop if the group is full.
                break;
            }
        }
    }

    for (auto& group : groups) { // Final pass to ensure groups meet the required size.
        unordered_map<string, int> emailCount; // Map to count email occurrences.
        if (group.size() >= groupSize) {
            continue;
        }
        for (int i = 0; i < group.size(); i++) {
            if (group.size() >= groupSize) {
                break;
            }
            auto tempPreferences = preferences[group[i]];
            for (auto preference : tempPreferences) {
                if (group.size() >= groupSize) {
                    break;
                }
                if (grouped.count(preference) >= 1) {
                    continue;
                }
                if (emailCount.count(preference) == 0) {
                    emailCount[preference] = 1;
                }
                else {
                    emailCount[preference]++;
                }
            }
        }

        vector<pair<int, string>> sortedEmails;
        for (const auto& entry : emailCount) {
            sortedEmails.push_back({ entry.second, entry.first });
        }

        // Sort the emails based on their frequency in descending order
        sort(sortedEmails.rbegin(), sortedEmails.rend());

        // Add the most popular emails to the group until it reaches the required size
        for (auto& email : sortedEmails) {
            if (group.size() >= groupSize) {
                break;
            }
            if (grouped.count(email.second) == 0) {
                group.push_back(email.second);
                grouped.insert(email.second);
            }
        }
    }

    for (auto& group : groups) { // Final attempt to fill groups with remaining emails.
        if (group.size() >= groupSize) {
            continue;
        }
        while (group.size() < groupSize && !remainingEmails.empty()) {
            randEmail = (rand() % remainingEmails.size());
            group.push_back(remainingEmails[randEmail]);
            grouped.insert(remainingEmails[randEmail]);
            remainingEmails.erase(remainingEmails.begin() + randEmail);
        }
    }

    // Output groups to the file
    file.open(filename2, ios::out); // Open the output file for writing.
    for (auto& group : groups) { // Write each group to the file.
        for (int i = 0; i < group.size(); i++) {
            file << group[i] << ",";
        }
        file << endl;
    }
}