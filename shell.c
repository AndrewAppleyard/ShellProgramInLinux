#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#define BSIZE 1024

void signit(){
    fflush(stdout);
}

int main(int argc, char *argv[]){
    char buffer[BSIZE];
    pid_t child;
    int status;
    char *args[BSIZE/2 + 1];
    char * inputfile = NULL;
    char * outputfile = NULL;
    int input,output;
    int pipef[2];
    signal(SIGINT, signit);

    while(1){
        printf("shell> ");
        fgets(buffer,BSIZE,stdin);
        buffer[(strlen(buffer)-1)]='\0';  //Remove NewLine

        inputfile = NULL;
        outputfile = NULL;
        char * input_redirect = strchr(buffer, '<');
        char * output_redirect = strchr(buffer, '>');
        if(input_redirect != NULL){
            *input_redirect = '\0';
            inputfile = strtok(input_redirect + 1, " ");
            inputfile = strtok(inputfile, " ");
        }
        if(output_redirect != NULL){
            *output_redirect = '\0';
            outputfile = strtok(output_redirect + 1, " ");
            outputfile = strtok(outputfile, " ");
        }

        char * token = strtok(buffer, " ");
        int i = 0;
        while(token != NULL){
            args[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        args[i]=NULL;

        //** Internal Command **
        if(strcmp(buffer,"exit")==0){
            break;
        } else if (strcmp(args[0], "cd") == 0){
            if (args[1] != NULL){
                if (chdir(args[1]) != 0){
                    perror("cd");
                }
            } else {
                chdir(getenv("HOME"));
            }
            continue;
        }

        if(pipe(pipef) == -1){
            perror("pipe");
            exit(1);
        }

        //** External Command **
        child = fork();
        if(child==0){
            if(inputfile != NULL){
                input = open(inputfile, O_RDONLY);
                if(input == -1){
                    perror("open");
                    exit(1);
                }
                dup2(input, STDIN_FILENO);
                close(input);
            }
            if(outputfile != NULL){
                output = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if(output == -1){
                    perror("open");
                    exit(1);
                }
                dup2(output, STDOUT_FILENO);
                close(output);
            }
            close(pipef[1]);
            dup2(pipef[0], STDIN_FILENO);
            close(pipef[0]);

            if(outputfile == NULL){
                dup2(pipef[1], STDOUT_FILENO);
                close(pipef[0]);
                close(pipef[1]);
            }
            
            //*** Child Process ***
            execvp(args[0],args);
            perror("execvp");
            exit(1);
        }else{
            //*** Parent Process **
            waitpid(child,&status,0);
            
        }

    }
    
    return 0;
}