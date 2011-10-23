/* Programmer: Arun Augustine */
/* Project   : basic_shell */

#define MAX_USER_INPUT 1024
#define MAX_TOKEN_SIZE 64
#define MAX_TOKENS 32

const int STDIN = 0, STDOUT = 1;
typedef enum {
  ALL_OK,
  FORK_FAILED,
  EXECVP_FAILED,
  FILE_ERROR,
  FILE_ERROR_CLOSE,
  FILE_ERROR_DUP2,
  ILLEGAL_REDIRECTION
} aa_status_codes_t;

//int aa_parse_into_tokens(char *);
//aa_status_codes_t execute_command (char* lexed_user_input[MAX_TOKENS]);
//aa_status_codes_t execute_input_redirection_command(char *tokens[], int fileConnect);
//aa_status_codes_t execute_output_redirection_command(char *tokens[]);
//aa_status_codes_t execute_advanced_command(char *tokens[]);
//aa_status_codes_t execute_advanced_command(char *tokens[], int *specialCharCount);
//aa_status_codes_t my_tokenize(char *tokens[], int size, char *);
//aa_status_codes_t my_tokenize(char *tokens[], char *userInput);
aa_status_codes_t my_tokenize(char *tokens[], char *userInput, int *specialCharCount);
int temp2token(char *tokens[], int tokenIndex, char * tempToken);
int copy_special_char_to_token(char *tokens[], int tokenIndex, char *specialchar);
