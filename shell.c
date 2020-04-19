#include "shell.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

int call_redirected(struct command *c) {
  pid_t proc = fork();
  if (proc < 0) {
    perror("fork failed");
    return 0;
  } else if (proc == 0) {
    // Set up redirection of process
    if (c->in_redir != NULL) {
      int fd = open(c->in_redir, O_RDONLY, 0666);
      if (fd < 0) {
        perror("Problem opening file");
        exit(1);
      } else {
        // file descriptor 1 = stdout
        int filename = dup2(fd, 0);
        // Handle file duplication errors
        if (filename < 0) {
          perror("error duplicating files");
          exit(1);
        }
        // execute redirected program
        execvp(c->args[0], c->args);
        perror("execvp returned when it is not supposed to");
        exit(1);
      }
    } else if (c->out_redir != NULL) {
      int fd = open(c->out_redir, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (fd < 0) {
        perror("Problem opening file");
        exit(1);
      } else {
        // file descriptor 1 = stdout
        int filename = dup2(fd, 1);
        // Handle file duplication errors
        if (filename < 0) {
          perror("error duplicating files");
          exit(1);
        }
        // execute redirected program
        execvp(c->args[0], c->args);
        perror("execvp returned when it is not supposed to");
        exit(1);
      }
    } else {
      int fd = open("junk.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (fd < 0) {
        perror("Problem opening file");
        exit(1);
      } else {
        // file descriptor 1 = stdout
        int filename = dup2(fd, 1);
        // Handle file duplication errors
        if (filename < 0) {
          perror("error duplicating files");
          exit(1);
        }
        // execute redirected program
        execvp(c->args[0], c->args);
        perror("execvp returned when it is not supposed to");
        exit(1);
      }
    }
  } else {
    // wait for child process to finish
    int exit_status = 0;
    pid_t y = waitpid(proc, &exit_status, 0);
    if (y < 0) {
      perror("wait returned negative in child process");
      return 0;
    }
    if (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == 0) {
      return 1;
    } else {
      return 0;
    }
  }
};

void control_logic(struct command *c) {
  if (strcmp(c->args[0], "cd") == 0) {
    if (sizeof(c->args) > 1) {
      chdir(c->args[1]);
    } else {
      if (!chdir(getenv("HOME"))) {
        perror("user has removed home environment variable");
      }
    }
  } else if (strcmp(c->args[0], "setenv") == 0) {
    if (sizeof(c->args) == 2) {
      unsetenv(c->args[1]);
    } else if (sizeof(c->args) == 3) {
      setenv(c->args[1], c->args[2], 1);
    } else if (sizeof(c->args) == 1) {
      perror("failure to setenv with one argument");
    }
  } else if (strcmp(c->args[0], "exit") == 0) {
    free_command(c);
    exit(0);
  } else {
    // redirect to system command
    call_redirected(c);
  }
};

int main(int argc, char **argv) {
  // script mode
  if (argc == 2) {
    // buffer for command input 512 bytes
    char cmd_line[512];
    while (1) {
      FILE *script = fopen(argv[1], "r");
      if (script == NULL) {
        perror("could not open script file");
        exit(1);
      }

      fgets(cmd_line, sizeof(cmd_line), stdin);

      /*
          while (getline(&line, &len, in_file)) {
            struct command *c = parse_command(line);

            if (c->args[0] != NULL) {
              control_logic(c);
              free_command(c);
            }
          }*/
    }
  }

  // keyboard mode
  if (argc < 2) {
    // buffer for command input 512 bytes
    char cmd_line[512];
    while (1) {
      fprintf(stderr, "shell>");

      // take in user input
      fgets(cmd_line, sizeof(cmd_line), stdin);

      struct command *c = parse_command(cmd_line);
      if (c->args[0] != NULL) {
        control_logic(c);

        free_command(c);
      } else {
        free_command(c);
      }
    }
  }

  return 0;
}