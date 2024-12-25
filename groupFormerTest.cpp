#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>

using namespace std;

void FormGroups(const string filename1, const string filename2, int groupSize, vector<string> emails){
    vector<vector<string>> groups;
    unordered_map<string, vector<string>> preferences;
    for (auto email : emails){
        preferences[email] = {};
    }
    fstream file;
    file.open(filename1, ios::in);
    string line, tempEmail;
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
    for (auto email:emails){
        cout << email << " ";
        for (auto pref:preferences[email]){
            cout << pref << " ";
        }
        cout << endl;
    }
}

int main(){
    vector<string> emails = {
        "hazhar.bese24seecs@seecs.edu.pk", "ashoaib.bese24seecs@seecs.edu.pk", 
        "sabdullah.bese24seecs@seecs.edu.pk", "test@test.com", 
        "mumughal.bese24seecs@seecs.edu.pk", "sbukhari.bese24seecs@seecs.edu.pk", 
        "arehman.bese24seecs@seecs.edu.pk", "mrubait.bese24seecs@seecs.edu.pk", 
        "msafiullah.bese24seecs@seecs.edu.pk", "smasroor.bese24seecs@seecs.edu.pk", 
        "zawan.bese24seecs@seecs.edu.pk", "mbilal1.bese24seecs@seecs.edu.pk", 
        "shassan.bese24seecs@seecs.edu.pk", "fzunaira.bese24seecs@seecs.edu.pk", 
        "jahmad.bese24seecs@seecs.edu.pk", "hahmed.bese24seecs@seecs.edu.pk", 
        "amughal.bese24seecs@seecs.edu.pk", "hfurqan.bese24seecs@seecs.edu.pk", 
        "mirfan.bese24seecs@seecs.edu.pk", "awasif.bese24seecs@seecs.edu.pk", 
        "bahmed.bese24seecs@seecs.edu.pk", "biahmad.bese24seecs@seecs.edu.pk", 
        "aakbar.bese24seecs@seecs.edu.pk", "mtalha.bese24seecs@seecs.edu.pk", 
        "sqaiser.bese24seecs@seecs.edu.pk", "mnadeem.bese24seecs@seecs.edu.pk", 
        "mtarar.bese24seecs@seecs.edu.pk", "dkhalid.bese24seecs@seecs.edu.pk", 
        "aimran.bese24seecs@seecs.edu.pk", "sali.bese24seecs@seecs.edu.pk", 
        "shakeela.bibi@seecs.edu.pk", "momina.moetesum@seecs.edu.pk"
    };

    FormGroups("preferences.csv", "groups.csv", 3, emails);

}