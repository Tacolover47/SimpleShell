#include "shell.h"

#include <errno.h>
#include <fcntl.h>      // open
#include <signal.h>     // signal
#include <stdio.h>      // perror, fgets
#include <stdlib.h>     // exit
#include <string.h>     // strcmp
#include <sys/stat.h>   // open
#include <sys/types.h>  // fork, open, waitpid
#include <sys/wait.h>   // waitpid
#include <unistd.h>     // fork, execvp

extern char **environ;

// allows the use of system programs
void call_redirected(struct command *c) {
  // fork process
  pid_t proc = fork();
  if (proc < 0) {
    perror("fork failed to create child process");
    return;
  } else if (proc == 0) {
    // Set up redirection of process
    if (c->in_redir) {
      int in_fd = open(c->in_redir, O_RDONLY, 0666);
      if (in_fd < 0) {
        perror("Error opening input redirection file");
        exit(1);
      }
      // Handle file duplication errors
      if (dup2(in_fd, 0) < 0) {
        perror("Error duplicating file for input");
        exit(1);
      }
    }
    if (c->out_redir) {
      int out_fd = open(c->out_redir, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (out_fd < 0) {
        perror("Error opening output redirection file");
        exit(1);
      }
      // Handle file duplication errors
      if (dup2(out_fd, 1) < 0) {
        perror("error duplicating file for output");
        exit(1);
      }
    }
    // execute redirected program
    execvp(c->args[0], c->args);
    perror("execvp returned when it is not supposed to");
    exit(1);
  } else {
    // ignore control+C until child finishes
    signal(SIGINT, SIG_IGN);
    // wait for child process to finish
    int exit_status = 0;
    if (waitpid(proc, &exit_status, 0) < 0) {
      perror("waitpid returned negative in child process");
      exit(1);
    }
    // reset interupt back to default
    signal(SIGINT, SIG_DFL);
    if (WIFEXITED(exit_status)) {
      if (WEXITSTATUS(exit_status) != 0) {
        perror("command exited with a status:");
      }
    }  // TODO: HANDLE WIFSIGNALED
    if (WIFSIGNALED(exit_status)) {
      perror("child terminated");
    }
  }
};

// determines which command to run
void control_logic(struct command *c) {
  if (strcmp(c->args[0], "cd") == 0) {
    if (c->args[1] == NULL) {
      // no args, try to change to home directory
      if (getenv("HOME")) {
        if (chdir(getenv("HOME")) == -1) {
          perror("failed to change directory to home");
        }
      } else {
        // error if there is no HOME variable
        perror("unable to locate home variable");
      }
    } else {
      if (chdir(c->args[1]) == -1) {
        perror("failed to change directory");
      }
    }
  } else if (strcmp(c->args[0], "setenv") == 0) {
    if (c->args[1] != NULL) {
      if (unsetenv(c->args[1]) == -1) {
        perror("Could not unset environment variable");
      } else if (c->args[2] != NULL) {
        if (setenv(c->args[1], c->args[2], 1) == -1) {
          perror("Could not setenv");
        }
      }
    } else {
      perror("failure to setenv with one argument");
    }
  } else if (strcmp(c->args[0], "exit") == 0) {
    // exit without an error should free command *c
    free_command(c);
    exit(0);
    perror("failed to exit");
  } else if (c->args[0] == NULL) {
    // do nothing
  } else {
    // redirect to system command
    call_redirected(c);
  }
};

int main(int argc, char **argv) {
  FILE *input_stream;  // stream for script input
  char line[512];      // 512 byte buffer for command

  // script input
  if (argc == 2) {
    // open script file
    input_stream = fopen(argv[1], "r");
    if (input_stream == NULL) {
      perror("Could not open input file");
      exit(1);
    }
    // take input from script line by line
    // adapted next line from post on Cprogramming.com see references
    while (fgets(line, sizeof(line), input_stream) != NULL) {
      if (line == NULL) {
        perror("Failed reading command");
      }
      struct command *c = parse_command(line);
      // catch empty commands
      if (c->args[0] != NULL) {
        control_logic(c);
      }
      free_command(c);
    }
    // close the input file
    if (fclose(input_stream) == EOF) {
      // handle error closing input file
      perror("Failed to close input file");
    }
  } else if (argc == 1) {  // keyboard input
    fprintf(stderr, "shell> ");
    fflush(stdin);
    while (fgets(line, sizeof(line), stdin)) {
      struct command *c = parse_command(line);
      if (c->args[0] != NULL) {
        control_logic(c);
      }
      free_command(c);

      fprintf(stderr, "shell> ");
      fflush(stdin);
    }
  }
  return 0;
}
