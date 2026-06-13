#ifndef APPS_H
#define APPS_H

#include "stdint.h"

typedef uint32_t (*app_func_t)(char *args);

typedef struct {
  const char *name;
  const char *descr;
  app_func_t func;
} app_t;

extern app_t app_table[];
extern const uint32_t app_count;

#endif // APPS_H

