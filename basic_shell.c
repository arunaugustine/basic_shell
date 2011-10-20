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

#ifdef DEBUG
    printf("I am the parent process with pid: %d\n", (int)getpid());
#endif

    wait(&status);
    return ALL_OK;
  }

  if (pid == 0){//Child Process

#ifdef DEBUG
    printf("I am the child process with pid: %d, parent is:%d\n", (int)getpid(),(int)getppid());
#endif

    int exec_status;
    exec_status = execvp(command, tokens);
    if (exec_status == -1){
      //some error happened during execvp
      printf("Bad command :( Please try again\n");
      exit(EXECVP_FAILED);
    }
    else
      exit(ALL_OK);
  }
}
int temp2token(char *tokens[], int tokenIndex, char * tempToken){

    if(tokens[tokenIndex] == NULL){
	tokens[tokenIndex] = (char *) malloc(sizeof(char) * strlen(tempToken) +1);
      }
      else{
	bzero(tokens[tokenIndex], strlen(tokens[tokenIndex]));
      }
      if(strlen(tempToken)>0){
      strncpy(tokens[tokenIndex],tempToken, strlen(tempToken));
      }
      strncat(tokens[tokenIndex], "\0",1);
      bzero(tempToken,MAX_TOKEN_SIZE);
      return (strlen(tokens[tokenIndex]));
}

int copy_special_char_to_token(char *tokens[], int tokenIndex, char *specialchar)
{
      if(tokens[tokenIndex] == NULL){
	tokens[tokenIndex] = (char *)malloc(sizeof(char) * strlen(specialchar) +1 );
      }
      else {
	bzero(tokens[tokenIndex], strlen(tokens[tokenIndex]));
      }
      if(strlen(specialchar)>0){
      strncpy(tokens[tokenIndex],specialchar,strlen(specialchar));
      }
      strncat(tokens[tokenIndex], "\0",1);
      return (strlen(tokens[tokenIndex]));

}

//aa_status_codes_t my_tokenize(char *tokens[], int size, char *userInput, int specialCharCount[4])
aa_status_codes_t my_tokenize(char *tokens[], char *userInput)
{

  /* parse the array passed in the third argument into an array of strings */
  /* which is null terminated */
  int tokenIndex = 0;//track the index of tokens 
  int sizeOfCurrentToken = 0;
  char * cPos = userInput;
  char tempToken[MAX_TOKEN_SIZE];
  bzero(tempToken,MAX_TOKEN_SIZE);

  while(*cPos != '\0' && tokenIndex < MAX_TOKENS)
  {
	    //    if (*cPos == ' ' | *cPos == '\t'| *cPos == '<' | *cPos == '>' | *cPos == '|') {
    int n = 0;// Stores the number of characters written into tokens

    if (*cPos == ' ' | *cPos == '\t') {
      int n = temp2token(tokens, tokenIndex, tempToken);
      if(n > 0) tokenIndex++;
    }

    else if(*cPos == '<'){
      n = 0;
      n = temp2token(tokens, tokenIndex, tempToken);
      if (n > 0) tokenIndex++;

      n = 0;
      n = copy_special_char_to_token(tokens,tokenIndex,"<");
      if( n > 0) tokenIndex++;
    }
    else if(*cPos == '>'){
      n = 0;
      n = temp2token(tokens, tokenIndex, tempToken);
      if (n > 0) tokenIndex++;

      n = 0;
      if(*(cPos+1) == '>'){
	n = copy_special_char_to_token(tokens,tokenIndex,">>");
	if( n > 0) tokenIndex++;
	cPos++;
      }
      else {
	n = copy_special_char_to_token(tokens,tokenIndex,">");
	if( n > 0) tokenIndex++;
      }
    }
    else if(*cPos == '|'){
      n = 0;
      n = temp2token(tokens, tokenIndex, tempToken);
      if (n > 0) tokenIndex++;

      n = 0;
      n = copy_special_char_to_token(tokens,tokenIndex,"|");
      if( n > 0) tokenIndex++;
    }
#if 0
      if(tokens[tokenIndex] == NULL){
	tokens[tokenIndex] = (char *)malloc(sizeof(char) * strlen("<") +1 );
      }
      else {
	bzero(tokens[tokenIndex], strlen(tokens[tokenIndex]));
      }
      strncpy(tokens[tokenIndex],"<\0",strlen("<")+1);
#endif
#if 0
	else if(*cPos == '>'){
	  if(*(cPos+1) == '>'){
	  tokenIndex++;
	  tokens[tokenIndex] = (char *)malloc(sizeof(char) * strlen(">>") + 1);
	  strncpy(tokens[tokenIndex], ">>\0",strlen(">>") +1 );
	  }
	  else {
	    tokenIndex++;
	    tokens[tokenIndex] = (char *)malloc(sizeof(char) * strlen(">") + 1);
	    strncpy(tokens[tokenIndex], ">\0",strlen(">") + 1);
	  }
	  if(*cPos == '|'){
	  tokenIndex++;
	  tokens[tokenIndex] = (char *)malloc(sizeof(char) * strlen("<") +1 );
	  strncpy(tokens[tokenIndex],"|\0",strlen("|")+1);
	  }
	}
      }
#endif

    else {
      strncat(tempToken,cPos,1);
    }
    cPos++;
#ifdef DEBUG
    printf("cPos is %c\n", *cPos);
#endif
  }
  tokens[tokenIndex] = (char *)malloc(sizeof(char) * strlen(tempToken) + 1);
  if(strlen(tempToken)>0){
  strncpy(tokens[tokenIndex],tempToken, strlen(tempToken));
  }
  strncat(tokens[tokenIndex],"\0",1);
  return 0;
}//Function End

void free_tokens(char *tokens[])
{
int i;
  for (i = 0; tokens[i] != NULL; i++){
    bzero(tokens[i], strlen(tokens[i] + 1));
    tokens[i]=NULL;
    free(tokens[i]);
  }
}

int main ( void ) {

  char *p;
  char *tokens[MAX_TOKENS] = {0};
  char *savptr;//used in strtok_r function
  int i;
  aa_status_codes_t my_status;
  char *ptr; //points to beginning of userInput

  char cleanedUserInput[MAX_USER_INPUT];

  while (1) {
    /* Display the prompt */
    char userInput[MAX_USER_INPUT];
    printf("shell:");
    /* Read the user command */
    if (fgets(userInput, sizeof(userInput), stdin) != NULL){

      /* clean up the newline character */
      /* also clean up spaces or tabs in the end */
      if ((p = strchr(userInput, '\n')) != NULL){
	*p = '\0';
	for( p = p-1; (*(p) == ' ' | *(p) == '\t') ; p-- ){
	  *p = '\0';
	  if (p == userInput) break;
	}
      }

#ifdef DEBUG
      printf("The user entered: %s \n", userInput);
#endif

#if 0
      ptr = userInput;
      i = 0;
      while ((tokens[i] = strtok_r(ptr, " ",&savptr)) != NULL && i< MAX_TOKENS){
	      i++;
	      ptr = savptr;
      }
#endif
      int tokenize_status; 
      if (strlen(userInput)>0){
	tokenize_status = my_tokenize(tokens, userInput);
      }
      else continue;//user either entered newline or space or tab character.

#ifdef DEBUG    
      for(i = 0; i < MAX_TOKENS; i++){
	printf("The tokens number %d is %s\n", i, tokens[i]);
	if(tokens[i] == NULL) break;
      }
#endif

      my_status =  execute_command(tokens);
      free_tokens(tokens);
      if(my_status != ALL_OK){
#ifdef DEBUG
	printf("somekind of error occured. Handle it here\n");
#endif
      }
      else {
#ifdef DEBUG 
	printf("everything went smoothly\n");
#endif
      }
    }
    /* Handle Ctl-D */
    else {
      printf ("\nUser entered ctr-D. Exiting\n");
      return 0;
    }
  
  }

} // main
