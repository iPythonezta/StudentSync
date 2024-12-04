#include <iostream>
#include <crow.h>
#include <sqlite3.h>

using namespace std;

int main(void){


    crow::SimpleApp studentSync;

    CROW_ROUTE(studentSync, "/")([]() {
        return "Hello world";
        });

    studentSync.port(2028).run();

}

