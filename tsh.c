#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
// by max glover


char error_message[] = "An error has occurred (from MG)\n";
int nodeCount=0;

char* args[64];
char* paths[64]; 
int num_paths = 0;

void print_error() {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

Node *head = NULL; 


void insertAtBeginning(char *data) {
    Node *newNode = malloc(sizeof(Node));
    newNode->data = strdup(data);
    newNode->next = head;
    head = newNode;
    nodeCount++;

    if(nodeCount>50){
        Node *prev = NULL;
        Node *temp = head;
        while (temp->next != NULL) {
            prev = temp;
            temp = temp->next;
        }
        
        prev->next = NULL;
        free(temp->data);
        free(temp);
        
    }
}

void displayList() {
    Node *temp = head;
    while (temp != NULL) {
        printf("%s", temp->data);
        temp = temp->next;
    }
}



int main(int argc, char* argv[]) {
    

    int use_execvp = 0; 
    if (argc == 2) {
        if (strcmp(argv[1], "-execvp") == 0) {
            use_execvp = 1;
            printf("**based on your choice, execvp() will be used**\n");
        } else if (strcmp(argv[1], "-execlp") == 0) {
            printf("**based on your choice, execlp() will be used**\n");
        }
        else{
            print_error();
            exit(1);
        }
    } else if (argc > 2) {
        print_error();
        exit(1);
    }

    char* buffer = NULL;
    ssize_t bufferSize = 1024;

    while (1) {
        printf("tsh> ");

        getline(&buffer, &bufferSize, stdin);

        insertAtBeginning(buffer);

        char* token = strtok(buffer, " \t\n");

        int argnum=0;
        int fileind=-1;

        while (token != NULL && argc < 64) {
        args[argnum++] = token;
        if(strcmp(token, ">")==0){
            fileind=argnum;
        }
        token = strtok(NULL, " \t\n");
        }

        if(use_execvp==1){
            args[argnum]=NULL;

        }


        int original = dup(STDOUT_FILENO);

        if(fileind>=0){
            char *file = args[fileind];

            int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if(fd==-1){
                print_error();
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);

            args[fileind]= NULL;
            args[fileind - 1]= NULL;
        }


        if (strcmp(args[0], "exit") == 0) {
            if (argnum > 1) {
                print_error();
            }
            exit(0);
        } 

        else if (strcmp(args[0], "cd") == 0) {
            if (argnum != 2) {
                print_error();
            }
            else {
                if (chdir(args[1]) == -1) {
                    print_error();
                }
            }
        }

        else if (strcmp(args[0], "path") == 0) {
            if(argnum==1){
                printf("path is set to ");
                for (int i = 0; i < num_paths; i++) {
                    if(i+1==num_paths){
                        printf("%s", paths[i]);
                    }
                    else{
                        printf("%s:", paths[i]);
                    }
                }
                printf("\n");
            }
            else{
                num_paths = 0; 
                char* fullpath =strdup(args[1]);
                char* str =strdup(args[1]);
                char* token2 = strtok(str, ":");

               while(token2!=NULL){
                    paths[num_paths++] = strdup(token2); 
                    token2 = strtok(NULL, ":");
                }
            }
        }

        else if (strcmp(args[0], "history") == 0) {
            displayList(head);
        }

        else{
            pid_t pid = fork();
            if (pid == -1) {
                print_error();
            } 
            else if (pid == 0) {
                if(use_execvp==1){
                    int found=0;
                    for(int i=0; i<num_paths;i++){
                        char* dir= NULL; //= paths[i]+strdup("/")+strdup(args[0]);
                        strcpy(dir, paths[i]);
                        strcpy(dir, "/");
                        strcpy(dir, args[0]);
                        if(access(dir, X_OK)){
                            execvp(dir, args);
                            found=1;
                        }
                    }
                    if(found==0){
                        execvp(args[0], args);                   
                    }
                }
                else {
                    int found=0;
                    for(int i=0; i<num_paths;i++){
                        char* dir= NULL; //= paths[i]+strdup("/")+strdup(args[0]);
                        strcpy(dir, paths[i]);
                        strcpy(dir, "/");
                        strcpy(dir, args[0]);
                        if(access(dir, X_OK)){
                            execlp(dir, args[1], args[2], args[3], args[4], NULL);
                            found=1;
                        }
                    }
                    if(found==0){
                        execlp(args[0], args[1], args[2], args[3], args[4], NULL);
                    }
                }
                print_error();
                exit(1);
            } 
            else {
                wait(NULL); 
            }
        }
        dup2(original, STDOUT_FILENO);
        close(original);
    }

    return 0;
}


