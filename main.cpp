#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pwd.h>

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
        struct passwd *pw = getpwuid(getuid());
        const char *homedir;
        if ((homedir = getenv("HOME")) == NULL) {
            homedir = getpwuid(getuid())->pw_dir;
        }
        char wd[1000];
        getcwd(wd, sizeof(wd));
        string hello;
        hello = "~";
        if(strcmp(homedir, wd) < 0){
            hello += ("/" + split(wd, '/').back());
        }
        gid_t gid = getgid();
        if(long(gid) == 0)
            hello += "!";
        else
            hello += ">";
        cout << hello;

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
            if(pid == -1){
                perror("fork failed");
            }
            else if(pid == 0){
                if(command == "cd"){
                    if(args.size() > 2){
                        cout << "cd: too many arguments" << endl;
                    }
                    else if(args.size() == 1){
                        int err = chdir(homedir);
                        if(err == -1){
                            perror("cd");
                        }
                    }
                    else {
                      int err = chdir(args[1]);
                      if(err == -1){
                          perror("cd");
                      }
                    }
                }
                else if(command == "pwd"){
                    if(words.size() > 1){}
                    else{
                        getcwd(wd, sizeof(wd));
                        cout << wd << endl;
                    }
                }
                else if(command == "time"){}
                else{
//                    cout << words[0] << endl;
                    args.push_back(NULL);
                    execvp(args[0], &args[0]);
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
