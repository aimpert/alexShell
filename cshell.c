#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NRM "\x1B[0m"
#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define YEL  "\x1B[33m"
#define BLU  "\x1B[34m"
#define MAG  "\x1B[35m"
#define CYN  "\x1B[36m"
#define GRY  "\x1B[37m"
#define RESET "\x1B[0m"


static struct Command *cmdLog;
static struct EnvVar* head;
static int size;
static int argSize;
static char *color = "\x1B[32m\0"; // default colour
//static char *CL;
void init(char *path);
void defaultState(char *path);
void parseLine(char *userStr);
void sortFunction(char *command, char *args[]);
int createVariable(char *command, char *args[]);
void builtInFunctions(char *command, char *args[]);
int printFunction(char *command, char *args[]);
int logFunction();
void logCmd(char *command, int returnCode);
int nonBuiltInFunction(char* command, char* args[]);
int changeTheme(char* command, char* args[]);

// linked list for global environment variables
struct EnvVar {
  char *name;
  char *value;
  struct EnvVar *next;
};

// for logging
struct Command {
  char *name;
  struct tm time;
  int code;
};



int main( int argc, char *argv[] ) {

  char *def = "0";

  // arv[0] is the name of the main file, so we need
  // argc at 2 for script mode
  if (argc == 2) {
    char filePath[PATH_MAX];
    getcwd(filePath, sizeof(filePath));
    strcat(filePath, "/");
    strcat(filePath, argv[1]);
    printf("%s \n", filePath);
    init(filePath);

  }
  // condition for when user passes an extra argument
  else if (argc > 2) {
    printf("Too many arguments.\n");
    exit(0);
  }
  // interactive mode
  else {
    init(def);
  }




   return 0;
}


void init(char *path) {
  size = 1;
  cmdLog = malloc(size * sizeof(*cmdLog)); // create space for our log struct
  head = NULL; // for our linked list



  defaultState(path);
}

// our while loop that keeps the program idle and waiting for an input
void defaultState(char *path) {

// interactive mode
if (path[0] == '0') {

  while (true) {
    char userStr[256];
    printf("%s", color);
    printf("alexShell$ " RESET);
    fgets(userStr, 256, stdin);
    printf("%s", color);
    //printf("Printing: %s\n", userStr);
    parseLine(userStr);
    //printf("after parse\n");

  }
}

// script mode
else {
  FILE *file;
  file = fopen(path, "r");

  while (true) {
    char userStr[256];
    printf("%s", color);
    if( fgets (userStr, 256, file)!=NULL ) {
    parseLine(userStr);
}
    else {
      exit(0);
    }


  }

  fclose(file);

}

}

//parsing functions that splits user input into tokens and then sends it to the sort function
void parseLine(char *userStr) {

  argSize = 0;

  if (userStr[0] == '$') {


    char *token = strtok(userStr, "=\n");

    char *command = (char*)malloc(sizeof(char) * sizeof(token));
    strcpy(command, token);

    char **args = (char**)malloc(sizeof(char));

    args = (char**)realloc(args, sizeof(char*));
    args[0] = (char*)malloc(sizeof(char) * sizeof(token));

    token = strtok(NULL, "\n");
    strcpy(args[0], token);


    logCmd(command, createVariable(command, args));

  }

  else {


    char *token = strtok(userStr, " \n");

    char *command = (char*)malloc(sizeof(char) * sizeof(token));

    char **args = (char**)malloc(sizeof(char));

    int i = 0;

    args[i] = (char*)malloc(sizeof(char) * sizeof(token));

    while(token != NULL)
    {
            args = (char**)realloc(args, sizeof(char*) * (i+1));
            args[i] = (char*)malloc(sizeof(char) * sizeof(token));

            if (i == 0) {
              strcpy(command, token);

              //printf("inside i==0: %s\n", command);
            }
            else {
              argSize++;
              strcpy(args[i-1], token);
              //printf("copied: %d ", i);
              //printf("%s\n", token);
            }

            token = strtok(NULL, " \n");

            i++;

    }

    sortFunction(command,args);
    free(args);

  }


}

//sort function that scans the command token and deciphers whether it is a built in or non-built in command
void sortFunction(char *command, char *args[]) {

  char arr[5][10] = {"exit", "print", "vprint", "log", "theme"};


  for (int i = 0; i < 5; i++) {
        if (strcmp(arr[i], command) == 0) {

            builtInFunctions(command, args);
            return;
        }
      }

  logCmd(command, nonBuiltInFunction(command, args));

}


//determines which built in command is being called
void builtInFunctions(char *command, char *args[]) {
  if (strcmp(command, "exit") == 0) {
    printf("Program closed.\n");
    exit(0);
  }

  else if (strcmp(command, "print") == 0) {
    logCmd(command, printFunction(command, args));
  }

  else if (strcmp(command, "vprint") == 0) { // function to list all environment variables
    struct EnvVar *ptr = head;

    while(ptr != NULL) {
       printf("%s = %s\n",ptr->name,ptr->value);
       ptr = ptr->next;
    }
  }

  else if (strcmp(command, "theme") == 0) { // for changing colours

    logCmd(command, changeTheme(command, args));

  }

  else if (strcmp(command, "log") == 0) {
    logCmd(command, logFunction());
  }

}


// function for creating a global variable using simple linked list
int createVariable(char *command, char *args[]) {


    struct EnvVar* new_EnvVar = (struct EnvVar*) malloc(sizeof(struct EnvVar));

    new_EnvVar->name  = command;
    new_EnvVar->value  = args[0];

    new_EnvVar->next = head;

    head = new_EnvVar;

    return 0;
}

int printFunction(char *command, char *args[]) {

    struct EnvVar* ptr = head;;
    int cmpFlg = -1;
    for (int i = 0; i < argSize; i++) {

      if (args[i][0] == '$') { // we need to check if one of the args passed is a global variable
        while(ptr != NULL) {
          if (strcmp(args[i], ptr->name) == 0) {
            printf("%s ", ptr->value);
            cmpFlg++;
            break;
          }



        ptr = ptr->next;
        }
      if (cmpFlg < 0) {
        printf("A variable %s does not exist.\n", args[i]);
        return -1;
      }
      //printf("Inside the thingy\n");
      ptr = head;
      }
      else { // otherwise print args passed
        printf("%s ", args[i]);
      }

    }
    printf("\n");
    return 0;
}

// function that prints all logged commands with their given times
int logFunction() {


    for (int i = 0; i < size-1; i++) {
      printf("%s", asctime(&cmdLog[i].time));
      printf("%s   ", cmdLog[i].name);
      printf("%d\n", cmdLog[i].code);
    }

    return 0;
}

// logCmd that attaches to other commands for logging
void logCmd(char *command, int returnCode) {
    time_t rawtime;
    time(&rawtime);

    cmdLog[size-1].name = command;
    cmdLog[size-1].code = returnCode;
    memcpy(&cmdLog[size-1].time, localtime( &rawtime ), sizeof(struct tm));

    size++;
    cmdLog = (struct Command*) realloc(cmdLog, size * sizeof(*cmdLog));

}

// programs ran from /bin/ aka non built in functions
int nonBuiltInFunction(char* command, char* args[]) {
  char fpath[5+sizeof(command)] = "/bin/";
  strcat(fpath, command);
  char buf[100];

// warning: band aid fix. this was the best solution under time constraint
// the way args[] usually works is args[0] is the name of the command and
// everything after are arguments. Below I am creating a new array of strings
// to properly my own args pass into execvp()

  char *argv[argSize+2];
  argv[0] = command;
  if (argSize > 0) {
    for (int i = 0; i < argSize; i++) {
      argv[i+1] = args[i];
    }
    argv[argSize + 2] = NULL;
  }
  else {
    argv[1] = NULL;
  }

  int n;
  int fds[2];
  pipe(fds);

  pid_t child_pid = fork();

  if (child_pid == 0) {
    dup2(fds[1],1);
    dup2(fds[1],2);
    close(fds[0]);

    if (execvp(fpath, argv) == -1) {
        printf("Command not recognized.\n");
    }
    exit(0);
  }

  else {
    close(fds[1]);
    n = read(fds[0], buf, sizeof(buf));
    write(1, buf, n);
  }

  return 0;
}

// the worst function I've ever written
int changeTheme(char* command, char* args[]) {

  char arr[8][10] = {"white\0", "red\0", "green\0", "yellow\0", "blue\0", "magenta\0", "cyan\0", "grey\0"};

  char *norm = "\x1B[0m";
  char *red =  "\x1B[31m";
  char *green =  "\x1B[32m";
  char *yellow =  "\x1B[33m";
  char *blue =  "\x1B[34m";
  char *magenta =  "\x1B[35m";
  char *cyan =  "\x1B[36m";
  char *grey = "\x1B[37m";

  int num;
  //char cor[7][10] = {"\x1B[31m\0", "\x1B[32m\0", "\x1B[33m\0", "\x1B[34m\0", "\x1B[35m\0", "\x1B[36m\0", "\x1B[37m\0"};
  for (int i = 0; i < 8; i++) {
        if (strcmp(arr[i], args[0]) == 0) {
            num = i;
        }
      }

// absolutely hate how I implemented this, but
// given time constraints this is what I went with
// as when I had these in arrays I was getting memory issues everywhere

switch (num) {
    case 0:
      color = norm;
      break;

    case 1:
      color = red;
      break;

    case 2:
      color = green;
      break;

    case 3:
      color = yellow;
      break;

    case 4:
      color = blue;
      break;

    case 5:
      color = magenta;
      break;

    case 6:
      color = cyan;
      break;

    case 7:
      color = grey;
      break;

    default:
      printf("Incorrect colour.\n");
      return -1;
}


  return 0;


}
