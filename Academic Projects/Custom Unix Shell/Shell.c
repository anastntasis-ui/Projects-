#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>



void redirectIn(char *fileName){
    int in=open(fileName,O_RDONLY);
    if (in==-1) {
        perror("open");
        exit(1);
    }
    dup2(in,0);
    close(in);
}

void redirectOut(char *fileName){
    int out=open(fileName,O_WRONLY|O_TRUNC|O_CREAT,0600);
    if (out==-1) {
        perror("open");
        exit(1);
    }
    dup2(out,1);
    close(out);
}

void redirectAppend(char *fileName) {
    int app=open(fileName,O_WRONLY|O_APPEND|O_CREAT,0600);
    if (app==-1) {
        perror("open");
        exit(1);
    }
    dup2(app,1);
    close(app);
}
void createPipe(char *left[], char *right[]);

void execute_commands(char *argv[]){
    for (int i = 0; argv[i]!=NULL; i++) {
        if (strcmp(argv[i],"<")==0) {
            redirectIn(argv[i+1]);
            argv[i]=NULL;
        } else if (strcmp(argv[i],">")==0) {
            redirectOut(argv[i+1]);
            argv[i]=NULL;
        }else if(strcmp(argv[i],">>")==0){
            redirectAppend(argv[i+1]);
            argv[i]=NULL;
        }else if(strcmp(argv[i],"|")==0){
            argv[i] = NULL; 
            createPipe(argv, &argv[i+1]);
            return;
        
        }
    }
    

    pid_t pid=fork();
    if(pid<0){
        perror("fork");
        exit(1);
    }
    if(pid==0){
        if(execvp(argv[0], argv)<0){
            printf("%s command not found \n",argv[0]);
            exit(1);
        }
    }
    else {
        wait(NULL);
    }
    
}

void createPipe(char *left[], char *right[])
{
    int fd[2];
    pid_t pid1,pid2;

    if (pipe(fd)==-1) {
        perror("pipe");
        exit(1);
    }

    pid1 = fork();
    if (pid1==0) {
        
        close(fd[0]);              
        dup2(fd[1],STDOUT_FILENO); 
        close(fd[1]);
        execvp(left[0],left);
        perror("execvp left");
        exit(1);
    }

    pid2=fork();
    if (pid2==0) {
        
        close(fd[1]);              
        dup2(fd[0],STDIN_FILENO); 
        close(fd[0]);
        execvp(right[0],right);
        perror("execvp right");
        exit(1);
    }

    
    close(fd[0]);
    close(fd[1]);
    waitpid(pid1,NULL,0);
    waitpid(pid2,NULL,0);
}



void shell(){
    char*line=NULL;
    size_t blen=0;
    char *argv[100];
    

    while(1){
        printf("my_shell$ ");
        fflush(stdout);
        if(getline(&line,&blen,stdin)==-1) break;

        line[strcspn(line,"\n")]='\0';

        if(strcmp(line,"exit")==0) break;

        int argc=0;

        char *token=strtok(line," ");

        while(token!=NULL && argc<99){
            argv[argc++]=token;
            token=strtok(NULL," ");
            }
        argv[argc]=NULL;
        execute_commands(argv);
    }
    free(line);
}

int main(int argc,char* argv[]){
    shell();
}