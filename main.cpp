#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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
            vector<char *> args;
            for(int i = 0; i < words.size(); i++)
                args.push_back((char *)words[i].c_str());
            string command = args[0];

            pid_t pid = fork();
            if(pid == -1){}
            else if(pid == 0){
                if(command == "cd"){
                    if(words.size() > 2){}
                    else{
                      int err = chdir(args[1]);
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
//                    cout << words[0] << endl;
                    args.push_back(NULL);
                    execvp(command/*= args[0]*/, &args[0]);
                }

            }
            else{
                int code;
                wait(&code);
            }
        }

    }

    return 0;
}
