#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
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

    waitpid(pid, &status, 0);
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

void set_left_pipe_read(int *leftPipe)
{
dup2(leftPipe[0],STDIN);
close(leftPipe[0]); // Copy made above
close(leftPipe[1]); // We don't need this
}

void set_right_pipe_write(int *rightPipe)
{
dup2(rightPipe[1],STDOUT);
close(rightPipe[0]); // We don't need this
close(rightPipe[1]); //Copy made already
}

aa_status_codes_t execute_simple_command (char* tokens[], int *leftPipe, int *rightPipe, pid_t *pid_return)
{

  char * command;
  const int STDIN = 0, STDOUT = 1;
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
    *pid_return= pid;
    //waitpid(pid, &status, 0);
    return ALL_OK;
  }
  if (pid == 0){//Child Process
  if(leftPipe){
  set_left_pipe_read(leftPipe);
  }
  if(rightPipe){
  set_right_pipe_write(rightPipe);
  }
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

aa_status_codes_t execute_input_redirection_command(char *tokens[], int fileConnect, int *leftPipe, int *rightPipe, pid_t *pid_return)
{
  int index, pos;
  const int STDIN = 0, STDOUT = 1;
  int status;
  int infile;
  int outfile;
  char *internalTokens[MAX_TOKEN_SIZE];

  char *command;
  int appendFlag;

  pid_t child_pid, parent_pid, pid;

  pid = fork();

  if(pid < 0){ 
    fprintf(stderr, "fork failed\n");
    return (FORK_FAILED);
  }//fork failed

  if(pid > 0){ //parent
    int child_status;
    *pid_return = pid; 
    return (ALL_OK);
  }

  if(pid == 0){//child
    int exec_status;
    if(leftPipe){
    //Cannot have a leftPipe since a | cannot come before a < 
    //printf("Error: a | cannot preceed a <\n");
    ///return(ILLEGAL_REDIRECTION);
    set_left_pipe_read(leftPipe);
    }
    if(rightPipe){
      set_right_pipe_write(rightPipe);
    }
    for (index = 0; tokens[index]!=NULL; index++){
      if(strcmp(tokens[index],"<") == 0) {
	pos = index;
	break;
      }
    }

    for(index = 0; index < pos; index++){
      internalTokens[index] = tokens[index];
    }
    internalTokens[index] = NULL;

    if(tokens[pos+1] != NULL){
    infile = open(tokens[pos+1],O_RDONLY);
    }

    if(infile < 0) return (FILE_ERROR);
    else {
      if(close(STDIN)!=0) return(FILE_ERROR_CLOSE);
      else{
	if(dup2(infile,STDIN) < 0 ) return(FILE_ERROR_DUP2);
	else{//execute command

	  if(fileConnect == 1) //fileConnect flag is set next is > or >>
	  {//TODO add error checking code
	    if(strcmp(tokens[pos+2],">") == 0) {
	      appendFlag = 0;
	      outfile = open(tokens[pos+3], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	    }
	    else if (strcmp(tokens[pos+2], ">>") == 0) {
	      appendFlag = 1;
	      outfile = open(tokens[pos+3], O_CREAT | O_WRONLY | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
	    }

	    close(STDOUT);
	    dup2(outfile, STDOUT);
	  }	

	  command = tokens[0];
	  exec_status = execvp(command, internalTokens);
	  //status = execute_command(internalTokens);
	  if(exec_status < 0) return(EXECVP_FAILED);
	  close(infile);
	  close(outfile);
	  return(ALL_OK);
	}
      }
    }
  }
}

aa_status_codes_t execute_output_redirection_command(char *tokens[], int *leftPipe, int *rightPipe, pid_t *pid_return)
{
  int index, pos;
  const int STDIN = 0, STDOUT = 1;
  int status;
  int outfile;
  char *internalTokens[MAX_TOKENS];
  int appendFlag = 0;

  char *command;
  pid_t child_pid, parent_pid, pid;

  pid = fork();

  if(pid < 0){ 
    fprintf(stderr, "fork failed\n");
    return (FORK_FAILED);
  }//fork failed

  if(pid > 0){ //parent
    int child_status;
    *pid_return = pid;
    return (ALL_OK);
  }

  if(pid == 0){//child
    int exec_status;

    if(leftPipe){
      set_left_pipe_read(leftPipe);
    }
    if(rightPipe){
      // there cannot be a rightPipe since a | cannot come after > or >>
      //printf("Error: a | cannot come after a > or >>\n");
      //return(ILLEGAL_REDIRECTION);
      set_right_pipe_write(rightPipe);
    }

    for (index = 0; tokens[index]!=NULL; index++){
      if(strcmp(tokens[index],">") == 0) {
	pos = index;
	break;
      }
      else if (strcmp(tokens[index], ">>") == 0) {
      pos = index;
      appendFlag = 1;
      break;
      }
    }

    for(index = 0; index < pos; index++){
      internalTokens[index] = tokens[index];
    }
    internalTokens[index] = NULL;

    if(appendFlag == 1 && tokens[pos+1] != NULL){
      outfile = open(tokens[pos+1], O_CREAT | O_WRONLY | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
    }
    else {
      outfile = open(tokens[pos+1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    }

    if(outfile < 0) return (FILE_ERROR);
    else {
      if(close(STDOUT)!=0) return(FILE_ERROR_CLOSE);
      else{
	if(dup2(outfile,STDOUT) < 0 ) return(FILE_ERROR_DUP2);
	else{//execute command
	  command = tokens[0];
	  exec_status = execvp(command, internalTokens);
	  //status = execute_command(internalTokens);
	  if(exec_status < 0) return(EXECVP_FAILED);
	  close(outfile);
	  return(ALL_OK);
	}
      }
    }
  }

}

void clear_internal_tokens(char *internalTokens[]){
int i;
for (i = 0; internalTokens[i] != NULL; i++){
  internalTokens[i] = NULL;
  }
}

aa_status_codes_t execute_advanced_command(char *tokens[], int *specialCharCount)
{
  int status;
  int nPipes, nIR, nOR, nOAR;
  char * internalTokens[MAX_TOKENS];
  int leftPipe[2], rightPipe[2];
  int child_status;
  pid_t pid_return;

  nIR = *(specialCharCount);// # of input redirections
  nOAR = *(specialCharCount + 1);// # of output append redirections
  nOR = *(specialCharCount + 2);// # of output redirections
  nPipes = *(specialCharCount + 3); // # of pipes

  int nPipesProcessed = 0;
  int nSubCommands = 0;

  nSubCommands = nPipes + 1;

  int index, i = 0;
  int fileConnect = 0, readFromPipe = 0, writeToPipe = 0;
  int foundOutRed = 0, foundInRed = 0;

  int subCommandToExecute = 0;// index of subcommand to execute next starting from zero

  // if(nPipes > 0){
  //  pipe(rightPipe);
  //}
  if(nPipes == 0){
    for (index = 0; tokens[index] != NULL;) {

      if (strcmp(tokens[index], "<") == 0){
	internalTokens[i] = tokens[index]; // Copy the operator also into internalTokens

	i++;
	index++;
	internalTokens[i] = tokens[index]; //copy the next string which should be the file name
	if(tokens[index+1] != NULL){
	  if(strcmp(tokens[index+1], ">") == 0 | strcmp(tokens[index+1], ">>") == 0 ){
	    fileConnect = 1; //set fileConnect flag
	    i++;
	    index++;
	    internalTokens[i] = tokens[index]; // Copy the operator also into internalTokens
	    i++;
	    index++;
	    internalTokens[i] = tokens[index]; //copy the next string which should be the file name
	  }
	}
	i++;
	internalTokens[i] = NULL;
	status = execute_input_redirection_command(internalTokens, fileConnect, NULL, NULL, &pid_return);
	waitpid(pid_return, &child_status, 0);
	if (status == ALL_OK) {
	  index++;
	  clear_internal_tokens(internalTokens);
	  i = 0;
	}

      }

      else if (strcmp(tokens[index], ">") == 0 | strcmp(tokens[index], ">>") == 0 ){
	internalTokens[i] = tokens[index]; // Copy the operator also into internalTokens

	i++;
	index++;
	internalTokens[i] = tokens[index]; //copy the next string which should be the file name

	i++;
	internalTokens[i] = NULL;
	status = execute_output_redirection_command(internalTokens, NULL, NULL, &pid_return);
	waitpid(pid_return, &child_status, 0);
	if(status == ALL_OK) {
	  index++;
	  clear_internal_tokens(internalTokens);
	  i = 0; //reset i
	}
      }

      else {
	internalTokens[i] = tokens[index];
	i++;
	index++;
      }


    }
  }

#if 0
  else {//there are pipes involved
    for (index = 0; tokens[index] != NULL;) {

      if (strcmp(tokens[index], "<") == 0) {
	internalTokens[i] = tokens[index]; // Copy the operator also into internalTokens

	i++;
	index++;
	internalTokens[i] = tokens[index]; //copy the next string which should be the file name
	if(tokens[index+1] != NULL){
	  if(strcmp(tokens[index+1], ">") == 0 | strcmp(tokens[index+1], ">>") == 0 ){
	    fileConnect = 1; //set fileConnect flag
	    i++;
	    index++;
	    internalTokens[i] = tokens[index]; // Copy the operator also into internalTokens
	    i++;
	    index++;
	    internalTokens[i] = tokens[index]; //copy the next string which should be the file name
	  }
	}

	if (tokens[index+1] != NULL){
	  if(strcmp(tokens[index+1],"|") == 0){
	    pipe(rightPipe);
	    nPipesProcessed++;
	    index++;
	  }

	  i++;
	  internalTokens[i] = NULL;

	  if(nPipesProcessed == 1)//First set of tokens before a pipe
	  {
	    status = execute_input_redirection_command(internalTokens, fileConnect, NULL, rightPipe);//first command
	  }
	  else if (nPipesProcessed < nPipes) status = execute_input_redirection_command(internalTokens, fileConnect, leftPipe, rightPipe);//not first or last
	  else execute_input_redirection_command(internalTokens, fileConnect, leftPipe, NULL);// last command

	  if(nPipesProcessed > 1){
	    close(leftPipe[0]);
	    close(leftPipe[1]);
	  }
	  leftPipe[0] = rightPipe[0];
	  leftPipe[1] = rightPipe[1];

	  if (status == ALL_OK) {
	    index++;
	    clear_internal_tokens(internalTokens);
	    i = 0;
	  }
	}
      }

      else if (strcmp(tokens[index], ">") == 0 | strcmp(tokens[index], ">>") == 0 ){
	internalTokens[i] = tokens[index]; // Copy the operator also into internalTokens

	i++;
	index++;
	internalTokens[i] = tokens[index]; //copy the next string which should be the file name
	index++;

	if (tokens[index+1] != NULL){
	  if(strcmp(tokens[index+1],"|") == 0){
	    pipe(rightPipe);
	    nPipesProcessed++;
	    index++;
	  }
	  i++;
	  internalTokens[i] = NULL;


	  if(nPipesProcessed == 1)//First set of tokens before a pipe
	  {
	    status = execute_output_redirection_command(internalTokens, NULL, rightPipe);//first command
	  }
	  else if (nPipesProcessed < nPipes) status = execute_output_redirection_command(internalTokens, leftPipe, rightPipe);//not first or last
	  else execute_output_redirection_command(internalTokens, leftPipe, NULL);// last command

	  if(nPipesProcessed > 1){
	    close(leftPipe[0]);
	    close(leftPipe[1]);
	  }
	  leftPipe[0] = rightPipe[0];
	  leftPipe[1] = rightPipe[1];

	  //      status = execute_output_redirection_command(internalTokens);
	  if(status == ALL_OK) {
	    index++;
	    clear_internal_tokens(internalTokens);
	    i = 0; //reset i
	  }
	}
      }

      else if (strcmp(tokens[index],"|") == 0){
	pipe(rightPipe);
	nPipesProcessed++;

	//index++;
	//i++;
	internalTokens[i] = NULL;


	if(nPipesProcessed == 1)//First set of tokens before a pipe
	{
	  status = execute_simple_command(internalTokens, NULL, rightPipe);//first command
	}
	else if (nPipesProcessed < nPipes) status = execute_simple_command(internalTokens, leftPipe, rightPipe);//not first or last
	else status = execute_simple_command(internalTokens, leftPipe, NULL);// last command

#if 0
	if(nPipesProcessed > 0 && nPipesProcessed < nPipes){
	  close(leftPipe[0]);
	  close(leftPipe[1]);
	}
	leftPipe[0] = rightPipe[0];
	leftPipe[1] = rightPipe[1];
#endif

	//      status = execute_output_redirection_command(internalTokens);
	if(status == ALL_OK) {
	  index++;
	  clear_internal_tokens(internalTokens);
	  i = 0; //reset i
	}
      }
      //accumulation of other tokens
      else {
	internalTokens[i] = tokens[index];
	i++;
	index++;
      }


    }
    internalTokens[i] = NULL;
    status = execute_simple_command(internalTokens, leftPipe, NULL);
    close(leftPipe[0]);
    close(leftPipe[1]);
    clear_internal_tokens(internalTokens);
    i=0;

  }//when there are pipes involved
#endif

  else {//there are pipes involved
    clear_internal_tokens(internalTokens);
    int lrFlag = 0, rdFlag = 0;

    //Parse first sub-command into internalTokens
    for(index = 0 ; tokens[index] != NULL ; index++ ) {
      if(strcmp(tokens[index],"|") == 0 ) //if token is |
      {
	internalTokens[i] = NULL;
	i++;
	break;
      }
      else {
	internalTokens[i] = tokens[index];
	i++;
	//set appropriate flags
	if(strcmp(tokens[index], "<") == 0)
	{
	  lrFlag = 1;
	}
	else if((strcmp(tokens[index], ">") == 0) | (strcmp(tokens[index], ">>") == 0))
	{
	rdFlag = 1;
	}
      }
    }  //For loop

    //Let index now point to start of next sub-command
    index++;

    //Create the frist output pipe

    pipe(rightPipe);

    //Execute appropriate command according to flag with leftFlag set to NULL
    if(internalTokens[0] != NULL){
      if (lrFlag == 1 && rdFlag == 0){
	status = execute_input_redirection_command(internalTokens, 0, NULL, rightPipe, &pid_return);
      }
      else if(lrFlag == 0 && rdFlag == 1){
	status = execute_output_redirection_command(internalTokens, NULL, rightPipe, &pid_return);
      }
      else if(lrFlag == 0 && rdFlag == 0){
	status = execute_simple_command(internalTokens, NULL, rightPipe, &pid_return);
      }
      else {
      printf("Error: illegal redirection character placing\n");
      return(ILLEGAL_REDIRECTION);
      }
    }

    // The first pipe that was the right pipe for first command becomes 
    // left pipe for the next command
    leftPipe[0] = rightPipe[0];
    leftPipe[1] = rightPipe[1];

    waitpid(pid_return, &child_status, 0);
    subCommandToExecute++; //must be 1 now
    lrFlag = 0; //reset the flags 
    rdFlag = 0;

    //Now do similarly for the rest of the sub-commands except the last one
    for(subCommandToExecute = 1; subCommandToExecute < nSubCommands-1 ; subCommandToExecute ++)
    {

      //Clean up internalTokens and i
      i = 0;
      clear_internal_tokens(internalTokens);
      // Get the next subcommand into internalTokens

      while(tokens[index]!=NULL){
	if(strcmp(tokens[index],"|") == 0 ) //if token is |
	{
	  internalTokens[i] = NULL;
	  i++;
	  break;
	}
	else {
	  internalTokens[i] = tokens[index];
	  i++;
	  //set appropriate flags
	  if(strcmp(tokens[index], "<") == 0)
	  {
	    lrFlag = 1;
	  }
	  else if((strcmp(tokens[index], ">") == 0) | (strcmp(tokens[index], ">>") == 0))
	  {
	    rdFlag = 1;
	  }
	}
	index++;
      }

      //Let index now point to start of next sub-command
      index++;
      // internalTokens now has the current sub command
      //Create the right pipe for this command since all of them have one.

      pipe(rightPipe);

      //Execute appropriate command according to flag with leftFlag set to NULL
      if(internalTokens[0] != NULL){
	if (lrFlag == 1 && rdFlag == 0){
	  status = execute_input_redirection_command(internalTokens, 0, leftPipe, rightPipe, &pid_return);
	}
	else if(lrFlag == 0 && rdFlag == 1){
	  status = execute_output_redirection_command(internalTokens, leftPipe, rightPipe, &pid_return);
	}
	else if(lrFlag == 0 && rdFlag == 0){
	  status = execute_simple_command(internalTokens, leftPipe, rightPipe, &pid_return);
	}
	else {
	printf("illegal placing of redirection or pipe characters\n");
	return(ILLEGAL_REDIRECTION);
	}
      }

      close(leftPipe[0]); // close on parent
      close(leftPipe[1]); // Close on parent

      waitpid(pid_return, &child_status, 0);
      //subCommandToExecute++; 
      lrFlag = 0;
      rdFlag = 0;

      // The first pipe that was the right pipe for first command becomes 
      // right pipe for the next command
      leftPipe[0] = rightPipe[0];
      leftPipe[1] = rightPipe[1];


    }//big for loop for commands for all sub commands in the middle
    //index++; // Now point to the beginning of the last sub-command

    //All Pipes are now complete. Copy final tokens into internalTokens
    i = 0;
    clear_internal_tokens(internalTokens);
    while(tokens[index]!=NULL)
    {
	  internalTokens[i] = tokens[index];
	  //set appropriate flags
	  if(strcmp(tokens[index], "<") == 0)
	  {
	    lrFlag = 1;
	  }
	  else if((strcmp(tokens[index], ">") == 0) | (strcmp(tokens[index], ">>") == 0))
	  {
	    rdFlag = 1;
	  }
	  i++;
	  index++;
    }
    internalTokens[i] = NULL;
    //Execute Appropriate Command
    if(internalTokens[0] != NULL){
      if (lrFlag == 1 && rdFlag == 0){
	status = execute_input_redirection_command(internalTokens, 0, leftPipe, NULL, &pid_return);
      }
      else if(lrFlag == 0 && rdFlag == 1){
	status = execute_output_redirection_command(internalTokens, leftPipe, NULL, &pid_return);
      }
      else if(lrFlag == 0 && rdFlag == 0){
	status = execute_simple_command(internalTokens, leftPipe, NULL, &pid_return);
      }
      else {
	printf("illegal placing of redirection or pipe characters\n");
	return(ILLEGAL_REDIRECTION);
      }
    }
    // Close the left (final) pipe
    close(leftPipe[0]);
    close(leftPipe[1]);

    //wait(&child_status);
    waitpid(pid_return, &child_status, 0);
    subCommandToExecute++; //must be 1 now

}

  return(status);
}



int temp2token(char *tokens[], int tokenIndex, char * tempToken)
{

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
aa_status_codes_t my_tokenize(char *tokens[], char *userInput, int *specialCharCount)
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
      (*specialCharCount)++;
    }
    else if(*cPos == '>'){
      n = 0;
      n = temp2token(tokens, tokenIndex, tempToken);
      if (n > 0) tokenIndex++;

      n = 0;
      if(*(cPos+1) == '>'){
	n = copy_special_char_to_token(tokens,tokenIndex,">>");
	if( n > 0) tokenIndex++;
	(*(specialCharCount+1))++;
	cPos++;
      }
      else {
	n = copy_special_char_to_token(tokens,tokenIndex,">");
	if( n > 0) tokenIndex++;
	(*(specialCharCount+2))++;
      }
    }
    else if(*cPos == '|'){
      n = 0;
      n = temp2token(tokens, tokenIndex, tempToken);
      if (n > 0) tokenIndex++;

      n = 0;
      n = copy_special_char_to_token(tokens,tokenIndex,"|");
      if( n > 0) tokenIndex++;
      (*(specialCharCount+3))++;
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
  //char *savptr;//used in strtok_r function
  int i;
  aa_status_codes_t my_status;
  //char *ptr; //points to beginning of userInput

  //char cleanedUserInput[MAX_USER_INPUT];

  while (1) {
    /* Display the prompt */
    char userInput[MAX_USER_INPUT];
    int specialCharCount[4] = {0,0,0,0};
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
	tokenize_status = my_tokenize(tokens, userInput, specialCharCount);
      }
      else continue;//user either entered newline or space or tab character.

#ifdef DEBUG    
      for(i = 0; i < MAX_TOKENS; i++){
	printf("The tokens number %d is %s\n", i, tokens[i]);
	if(tokens[i] == NULL) break;
      }
      printf("the number of < is : %d\n",specialCharCount[0]);
      printf("the number of >> is : %d\n",specialCharCount[1]);
      printf("the number of > is : %d\n",specialCharCount[2]);
      printf("the number of | is : %d\n",specialCharCount[3]);

#endif
      if(specialCharCount[0] > 1){
	printf("Error: Utmost one input redirection allowed in one command\n");
	continue;
      }
      if(specialCharCount[1] > 1){
	printf("Error: Utmost one output (append) direction allowed in one command\n");
	continue;
      } 
      if(specialCharCount[2] > 1){
	printf("Error: Utmost one output redirection allowed in one command\n");
	continue;
      }
      if((specialCharCount[1] + specialCharCount[2]) > 1){
	printf("Error: Utmost one output redirection allowed in one command\n");
	continue;
      }
      if(specialCharCount[0] == 0 && specialCharCount[1] == 0 && specialCharCount[2] == 0 && specialCharCount[3] == 0){
      my_status =  execute_command(tokens);
      }
      else my_status = execute_advanced_command(tokens, specialCharCount);

      free_tokens(tokens);
      if(my_status != ALL_OK){
	if(my_status == FILE_ERROR){
	  printf("error in opening file\n");
	}
      if(my_status == FILE_ERROR_DUP2){
	  printf("error in dup2'ing file\n");
	}
      if(my_status == FILE_ERROR_CLOSE){
	  printf("error in closing'ing file\n");
	}
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
