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

    if(args.front() == "pwd"){
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

void conv(vector<string> components, int fout, char is_time){

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
            signal(2, SIG_DFL);
            dup2(fout,1);
            command(words);
        }
        else{
            tms st_cpu;
            tms en_cpu;
            clock_t st_time;
            clock_t en_time;
            if(is_time)
                st_time = times(&st_cpu);

            int status;
            wait(&status);
            dup2(oldstdout, 1);

            if(is_time){
                en_time = times(&en_cpu);
                cout << endl;
                cout << "real" << '\t' << (float)(en_time-st_time)/sysconf(_SC_CLK_TCK) << " s" << endl;
                cout << "user" << '\t' << (float)(en_cpu.tms_cutime - st_cpu.tms_cutime)/sysconf(_SC_CLK_TCK) << " s" << endl;
                cout << "sys" << '\t' << (float)(en_cpu.tms_cstime - st_cpu.tms_cstime)/sysconf(_SC_CLK_TCK) << " s" << endl;
            }
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
            signal(2, SIG_DFL);
            close(fd[0]);   //sends
            dup2(fd[1],1);

            command(words);
        }
        else{
            close(fd[1]); //receives
            dup2(fd[0],0);

//            int status;
//            wait(&status);
            dup2(oldstdout, 1);
            close(oldstdout);

            conv(components, fout, 0);
            dup2(oldstdin, 0);
            close(oldstdin);

        }
    }
}

int main() {
    signal(2,SIG_IGN);
    while(!feof(stdin)){

        hello();

        string input;
        getline(cin, input);
        if (input.empty())
                continue;

        vector<string> components = split(input, '|');

        char is_time = 0;
        vector<string> begin = split(components.front(), ' ');
        if(begin[0] == "time"){
            is_time = 1;
            begin.erase(begin.begin());
        }


        int fin = 0;
        int oldstdin = dup(0);

        if(begin.size() > 2)
            if(begin[begin.size()-2] == "<"){
                fin = open(begin.back().c_str(), O_RDONLY);
                dup2(fin,0);
                begin.pop_back();
                begin.pop_back();
            }
        string _begin;
        for(int i = 0; i < begin.size(); i++)
            _begin = _begin + begin[i] + ' ';
        components[components.size()-1] = _begin;

        int fout = 1;
        vector<string> end = split(components.back(), ' ');

        if(end.size() > 2)
            if(end[end.size()-2] == ">"){
                fout = open(end.back().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                end.pop_back();
                end.pop_back();
                string _end;
                for(int i = 0; i < end.size(); i++)
                    _end = _end + end[i] + ' ';
                components[components.size()-1] = _end;
            }



        if(components.size() > 1){
            conv(components, fout, 0);
//            continue;
        }
        else if(components.size() == 1){
            vector<string> words = split(components.front(), ' ');

            if(words.front() == "cd"){
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
                conv(components,fout, is_time);
            }
        }
        if(fin != 0){
            close(fin);
            dup2(oldstdin,0);
        }
        if(fout != 1)
            close(fout);


    }

    cout << endl;
    return 0;
}
