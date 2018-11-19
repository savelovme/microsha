#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pwd.h>
#include <glob.h>

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

string get_homedir(){
//    struct passwd *pw = getpwuid(getuid());
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    string ret = homedir;
    return ret;
}

void hello(){
    char wd[1000];
    getcwd(wd, sizeof(wd));
    string dir = wd;
    string hello;

    hello;
    if(dir > get_homedir()){
        hello += ("~/" + split(wd, '/').back());
    }
    else if(dir == get_homedir()){
        hello += "~";
    }
    else{
        hello += ("/" + split(wd, '/').back());
    }
    gid_t gid = getgid();
    if(long(gid) == 0)
        hello += "!";
    else
        hello += ">";
    cout << hello;
}

void command(vector<string> words){

    vector<string> _words;
    for(int i = 0; i < words.size(); i++){
        glob_t globbuf;
        globbuf.gl_offs = 2;
        int err = glob(words[i].c_str(), GLOB_NOMAGIC, NULL, &globbuf);
        if(err != 0)
            perror("glob_error");
        for(int j = 0; j < globbuf.gl_pathc; j++){
            _words.push_back(globbuf.gl_pathv[j]);
        }
        globfree(&globbuf);
    }
    vector<char *> args;
    for(int i = 0; i < _words.size(); i++){
        args.push_back((char *)_words[i].c_str());
    }

    if(args[0] == "pwd"){
        if(words.size() > 1){}
        else{
            char wd[1000];
            getcwd(wd, sizeof(wd));
            cout << wd << endl;
        }
    }
    else{
        args.push_back(NULL);
        execvp(args[0], &args[0]);
        perror(args[0]);
    }
}

void conv(vector<string> components, int fout){

    vector<string> words = split(components.front(), ' ');
    components.erase(components.begin());

    int oldstdin = dup(0);
    int oldstdout = dup(1);

    if(components.empty()){

        pid_t pid = fork();
        if(pid == -1){
            perror("fork failed");
        }
        if(pid == 0){
            dup2(fout,1);
            command(words);
        }
        else{
            int status;
            wait(&status);
            dup2(oldstdout, 1);
        }

    }
    else{
        int fd[2];
        int err = pipe(fd);
        if(err == -1){
            perror("pipe error");
        }

        pid_t pid = fork();
        if(pid == -1){
            perror("fork failed");
        }
        if(pid == 0){
            close(fd[0]);   //sends
            dup2(fd[1],1);

            command(words);
        }
        else{
            close(fd[1]); //receives
            dup2(fd[0],0);

            int status;
            wait(&status);
//            dup2(oldstdin, 0);
            dup2(oldstdout, 1);

            close(oldstdout);
            conv(components, fout);
            dup2(oldstdin, 0);
            close(oldstdin);
        }
    }
}

int find_el(vector<string> V, string el){
    int res = -1;
    if(el == "<")
        for(int i = 0; i < V.size(); i++){
            if(V[i] == "<" && i > 0){
                res = i;
                break;
            }
        }
    else if(el == ">")
        for(int i = V.size()-1; i >= 0; i--){
            if(V[i] == ">" && i < V.size()-1){
                res = i;
                break;
            }
        }
    return res;
}

int main() {

    while(!feof(stdin)){

        hello();

        string input;
        getline(cin, input);
        vector<string> components = split(input, '|');

        int fin = 0;
        int oldstdin = dup(0);
        string _begin;
        vector<string> begin = split(components.front(), ' ');
        int ind_beg = find_el(begin, "<");
        if(ind_beg != -1){
            for(int i = 0; i < begin.size(); i++)
                if(i != ind_beg-1 && i != ind_beg)
                    _begin = _begin + begin[i] + ' ';
            components[0] = _begin;
            fin = open(begin[ind_beg-1].c_str(), O_RDONLY);
            dup2(fin,0);
        }

        int fout = 1;
        string _end;
        vector<string> end = split(components.back(), ' ');
        int ind_end = find_el(end, ">");
        if(ind_end != -1){
            for(int i = 0; i < begin.size(); i++)
                if(i != ind_end && i != ind_end+1)
                    _end = _end + end[i] + ' ';
            components[components.size()-1] = _end;
            fout = open(end[ind_end+1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        }


        if(components.size() > 1){
            conv(components, fout);
//            continue;
        }
        else if(components.size() == 1){
            vector<string> words = split(components[0], ' ');
            if(words[0] == "cd"){
                if(words.size() > 2){
                    cout << "cd: too many arguments" << endl;
                }
                else if(words.size() == 1){
                    int err = chdir(get_homedir().c_str());
                    if(err == -1){
                        perror("cd");
                    }
                }
                else {
                    int err = chdir((words.back()).c_str());
                    if(err == -1){
                        perror("cd");
                    }
                }
            }
            else{
                conv(components,fout);
            }
        }
        if(fin != 1){
            close(fin);
            dup2(oldstdin,0);
        }
        if(fout != 1)
            close(fout);
    }

    return 0;
}
