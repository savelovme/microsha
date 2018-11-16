#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>

using namespace std;

vector<string> split(string str, char delimiter) {
    vector<string> internal;
    stringstream ss(str); // Turn the string into a stream.
    string tok;

    while(getline(ss, tok, delimiter)) {
        if(tok.size() > 0)
            internal.push_back(tok);
  }

  return internal;
}

int main() {
    char WORKING = 1;
    while(WORKING == 1){
        gid_t gid = getgid();
        if(long(gid) == 0)
            cout << "!";
        else
            cout << ">";

        string input;
        getline(cin, input);
        vector<string> components = split(input, '|');
        for(int i = 0; i < components.size(); i++){
            vector<string> words = split(components[i], ' ');
            string command = words[0];
            if(command == "cd"){
                if(words.size() > 2){}
                else{
                  int err = chdir(words[1].c_str());
//                  cout << "error = " << err << endl;
                }
            }
            else if(command == "pwd"){
                if(words.size() > 1){}
                else{
                    char wd[1000];
                    getcwd(wd, sizeof(wd));
                    cout << wd << endl;
                }
            }
            else if(command == "time"){}
            else{

            }
        }

    }

    return 0;
}
