#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/vga.h>

#include <builtin/apps.h>
#include <builtin/shell.h>

const static char *SHELL_MSG = ("This is KittenShell\n"
                                " Only for KittenOSv3\n");

#define ISBLANK(ch) (ch == ' ' || ch == '\t' || ch == '\n')
#define ISNTBLANK(ch) (ch != ' ' && ch != '\t' && ch != '\n')

static char *next_word(char *str) {
  if (str == NULL)
    return NULL;

  /* lstrip spaces */
  while (ISBLANK(*str) && *str != 0)
    ++str;

  /* Skip to first space */
  while (ISNTBLANK(*str) && *str != 0)
    ++str;
  if (*str == 0)
    return NULL;

  /* Replace spaces to '\0' */
  while (ISBLANK(*str) && *str != 0) {
    *str = 0;
    ++str;
  }

  /* End of string */
  if (ISBLANK(*str) || *str == 0)
    return NULL;

  return str;
}

int shell_app_func(int argc, char **argv) {
  char *input;
  size_t input_len;

  puts(SHELL_MSG);

  int status = 0;

  while (1) {
    /* Uneraseable prompt */
    printf("(%d) %s", status, PROMPT);
    vga_set_anchor();

    /* Read input */
    input = malloc(256);
    if (!getline(&input, &input_len, stdin)) {
      puts("Error while reading stdin");
      free(input);
      return ENOMEM;
    }

    /* Skip to the start of string */
    while (*input != 0 && (*input == ' ' || *input == '\t'))
      ++input;

    /* Remove '\n' at the end */
    char *raw_8 = input;
    while (*raw_8 != '\n' && *raw_8 != 0)
      ++raw_8;
    *raw_8 = 0;

    /* Parse CMD */
    char **argv = malloc(sizeof(char *) * 4);
    char *arg = input;
    size_t argc = 0;
    size_t mlc = 4;

    while (arg != NULL && ISNTBLANK(*arg)) {

      argv[argc] = arg;
      ++argc;

      if (argc >= mlc) {
        size_t nmlc = (mlc << 1);
        void *new_argv = malloc(nmlc * sizeof(char *));
        if (new_argv != NULL) {
          memcpy(new_argv, argv, argc * sizeof(char *));
          free(argv);
          mlc = nmlc;
          argv = new_argv;
        } else {
          break;
        }
      }
      arg = (char *)next_word(arg);
    }

    if (*argv[0] != 0) {
      if (strcmp(argv[0], "exit") == 0) {
        puts("Exiting shell");
        status = 0;
        break;
      }

      app_t app;
      int found = 0;
      for (size_t i = 0; i < app_table_size; ++i) {
        app = app_table[i];

        if (strcmp(argv[0], app.name) == 0) {
          found = 1;
          status = app.func(argc, argv);
          break;
        }
      }
      if (found == 0) {
        puts("Unknown command, try typing 'help'");
        status = 0xFF;
      }
    }

    /* Free memory */
    free(argv);
    free(input);
  }
  return SUCCESS;
}
