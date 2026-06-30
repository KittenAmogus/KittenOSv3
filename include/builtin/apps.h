#ifndef _APPS_H
#define _APPS_H

// App function
#include <stddef.h>
typedef int (*app_func_t)(int argc, char **argv);

// App struct
typedef struct {
  const char *name;
  const char *descr;
  app_func_t func;
} app_t;

extern const app_t app_table[];
extern const size_t app_table_size;

#endif // _APPS_H
