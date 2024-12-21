// #include <iostream>
// #include <fstream>
// #include <string>

// using namespace std;
// bool studentMarksToFile(string userEmail, string assesmentType, float obtainedMarks, int subjectId, string subName, int assesmentId){
//     fstream file;
//     string filePath = "student_marks/" + subName + "_" + to_string(subjectId) + ".csv";
//     file.open(filePath, ios::out | ios::app);
//     if (!file.is_open()) {
//         return false;
//     }
//     file << userEmail << "," << assesmentType << "," << assesmentId << "," << obtainedMarks << endl;
//     file.close();
//     return true;
// }

// int studentMarksFromFile(string userEmail, string assesmentType, int subjectId, string subName, int assesmentId){
//     fstream file;
//     string filePath = "student_marks/" + subName + "_" + to_string(subjectId) + ".csv";
//     file.open(filePath, ios::in);
//     if (!file.is_open()) {
//         return 0;
//     }
//     string line;
//     while (getline(file, line)) {
//         if (line.find(userEmail) != string::npos && line.find(assesmentType) != string::npos && line.find(to_string(assesmentId)) != string::npos) {
//             string marks = line.substr(line.find_last_of(",") + 1);
//             return stoi(marks);
//         }
//     }
// }
// int main(){
//     bool result = studentMarksToFile("lHq0X@example.com", "Assignment", 10, 1, "Assignment", 1);
//     int marks = studentMarksFromFile("lHq0X@example.com", "Assignment", 1, "Assignment", 1);
//     cout << marks << endl;
// }