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
    if (getline(&input, &input_len, stdin) < 0) {
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

    int argc = 16;
    char **argv = malloc(sizeof(char *) * argc);
    if (argv == NULL) {
      free(input);
      return ENOMEM;
    }

    argc = strsplit(input, ' ', argc, argv);
    if (argc < 1) {
      free(input);
      free(argv);
      puts("Failed to parse args");
      return EINVAL;
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
