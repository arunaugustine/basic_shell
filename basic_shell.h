/* Programmer: Arun Augustine */
/* Project   : basic_shell */

#define MAX_USER_INPUT 512
#define MAX_TOKEN_SIZE 32
#define MAX_TOKENS 16

typedef enum {
  ALL_OK,
  FORK_FAILED,
  EXECVP_FAILED
} aa_status_codes_t;

//int aa_parse_into_tokens(char *);
aa_status_codes_t execute_command (char* lexed_user_input[MAX_TOKENS]);
//aa_status_codes_t my_tokenize(char *tokens[], int size, char *);
aa_status_codes_t my_tokenize(char *tokens[], char *userInput);
int temp2token(char *tokens[], int tokenIndex, char * tempToken);
int copy_special_char_to_token(char *tokens[], int tokenIndex, char *specialchar);
