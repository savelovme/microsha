#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <errno.h>
#include <pwd.h>
#include <glob.h>
#include <signal.h>

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
        if(err != 0){
            perror("glob_error");
            exit(0);
        }
        for(int j = 0; j < globbuf.gl_pathc; j++){
            _words.push_back(globbuf.gl_pathv[j]);
        }
        globfree(&globbuf);
    }
    vector<char *> args;
    for(int i = 0; i < _words.size(); i++){
        args.push_back((char *)_words[i].c_str());
    }

    if(_words.front() == "pwd"){
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

void _conv(vector<vector<string>> components){

    vector<string> words = components.front();
    components.erase(components.begin());

    if(components.empty()){
        command(words);
        exit(0);
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
            exit(0);
        }
        else{
            close(fd[1]); //receives
            dup2(fd[0],0);

            _conv(components);

            int status;
            wait(&status);
        }
    }
}

void EXEC(vector<vector<string>> components, string in, string out, char is_time){

    if(components.empty())
        return;

    if(is_time){

        pid_t pid = fork();
        if(pid < 0)
            perror("fork error");
        else if(pid == 0){
            if(!in.empty()){
                close(0);
                if(open(in.c_str(), O_RDONLY) < 0){
                    perror("open error");
                    exit(0);
                }
            }
            if(!out.empty()){
                close(1);
               if(open(out.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666) < 0){
                   perror("open error");
                   exit(0);
               }
            }
            signal(2, SIG_DFL);
            _conv(components);
            exit(0);
        }
        else{

            tms st_cpu;
            tms en_cpu;
            clock_t st_time;
            clock_t en_time;
            st_time = times(&st_cpu);

            int status;
            wait(&status);

            en_time = times(&en_cpu);
            cout << endl;
            cout << "real" << '\t' << (float)(en_time-st_time)/sysconf(_SC_CLK_TCK) << " s" << endl;
            cout << "user" << '\t' << (float)(en_cpu.tms_cutime - st_cpu.tms_cutime)/sysconf(_SC_CLK_TCK) << " s" << endl;
            cout << "sys" << '\t' << (float)(en_cpu.tms_cstime - st_cpu.tms_cstime)/sysconf(_SC_CLK_TCK) << " s" << endl;

        }

    }
    else if(components.size() == 1 && components.front().front() == "cd"){
        vector<string> words = components.front();
        if(words.size() > 2){
            cout << "cd: too many arguments" << endl;
        }
        else if(words.size() == 1){
            int err = chdir(get_homedir().c_str());
            if(err < 0){
                perror("cd");
            }
        }
        else {
            int err = chdir((words.back()).c_str());
            if(err < 0){
                perror("cd");
            }
        }
    }
    else{
        pid_t pid = fork();
        if(pid < 0)
            perror("fork error");
        else if(pid == 0){
            if(!in.empty()){
                close(0);
                if(open(in.c_str(), O_RDONLY) < 0){
                    perror("open error");
                    exit(0);
                }
            }
            if(!out.empty()){
                close(1);
               if(open(out.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666) < 0){
                   perror("open error");
                   exit(0);
               }
            }
            signal(2, SIG_DFL);
            _conv(components);
            exit(0);
        }
        else
            wait(NULL);
    }
}

int main(){
    signal(2,SIG_IGN);
    while(!feof(stdin)){

        hello();

        string input;
        getline(cin, input);
        if (input.empty())
                continue;
        if(input.find('>') != input.rfind('>')){
            cout << "There could be only one '>'" << endl;;
            continue;
        }

        if(input.find('<') != input.rfind('<')){
            cout << "There could be only one '<'" << endl;
            continue;
        }

        vector<string> _comps = split(input, '|');
        input.clear();

        vector<vector<string>> components;
        for(int i = 0; i < _comps.size(); i++)
            components.push_back(split(_comps[i], ' '));
        _comps.clear();

        char is_time = 0;
        if(components.front().front() == "time"){
            is_time = 1;
            components[0].erase(components.front().begin());
        }

        string in, out;
        if(components.back().size() > 2)
            if(components.back()[components.back().size()-2] == ">"){
              out = components.back().back();
              components.back().pop_back();
              components.back().pop_back();
            }
        if(components.front().size() > 2)
            if(components.front()[components.front().size()-2] == "<"){
                in = components.front().back();
                components.front().pop_back();
                components.front().pop_back();
            }

        EXEC(components, in, out, is_time);
    }
    return 0;
}
