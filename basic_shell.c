#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "basic_shell.h"

aa_status_codes_t execute_command (char* tokens[MAX_TOKENS])
{
  
  char * command;
  pid_t child_pid;
  pid_t parent_pid;
  command = tokens[0];

  pid_t pid = fork();

  if (pid < 0){
    fprintf(stderr, "Fork failed\n");
    return FORK_FAILED;
  }
  if (pid > 0){ // Parent process
    int status;
    printf("I am the parent process with pid: %d\n", (int)getpid());
    wait(&status);
    return ALL_OK;
  }

  if (pid == 0){//Child Process
    //    printf("I am the child process with pid: %d, parent is:%d\n", (int)getpid(),(int)getppid());
    //    getchar();
    int exec_status;
    exec_status = execvp(command, tokens);
    exit(ALL_OK);
  }
}
int main ( void ) {

  char *p;
  char *tokens[MAX_TOKENS] = {0};
  char *savptr;//used in strtok_r function
  int i;
  aa_status_codes_t my_status;
  char *ptr; //points to beginning of userInput
  while (1) {
    /* Display the prompt */
  char userInput[MAX_USER_INPUT];
  printf("shell:");
    /* Read the user command */
  if (fgets(userInput, sizeof(userInput), stdin) != NULL){

    /* clean up the newline character */
    if ((p = strchr(userInput, '\n')) != NULL){
      *p = '\0';
    }

    //TODO: remove test printf
    printf("The user entered: %s \n", userInput);

    ptr = userInput;
    i = 0;
    while ((tokens[i] = strtok_r(ptr, " ",&savptr)) != NULL && i< MAX_TOKENS){
      i++;
      ptr = savptr;
      }

    
    /* for(i = 0; i < MAX_TOKENS; i++){ */
    /*   printf("The tokens number %d is %s\n", i, tokens[i]); */
    /*         if(tokens[i] == NULL) break; */
    /* } */
    my_status =  execute_command(tokens);
    if(my_status != ALL_OK){
      printf("somekind of error occured. Handle it here\n");
    }
    else printf("everything went smoothly\n");

    
  }
  /* Handle Ctl-D */
  else {
    printf ("User entered ctr-D. Exiting\n");
    return 1;
  }
  
}

} // main
