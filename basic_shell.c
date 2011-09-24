#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "basic_shell.h"


int main ( void ) {

  char *p;
  char *tokens[MAX_TOKENS] = {0};
  char *savptr;//used in strtok_r function
  int i;

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

    
    for(i = 0; i < MAX_TOKENS; i++){
      printf("The tokens number %d is %s\n", i, tokens[i]);
            if(tokens[i] == NULL) break;
    }

  }
  /* Handle Ctl-D */
  else {
    printf ("User entered ctr-D. Exiting\n");
    return 1;
  }
  
}

} // main
