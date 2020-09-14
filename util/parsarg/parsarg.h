#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  int    argc;
  char** argv;

  char* value;

  bool after_option;
} parsarg_t;

void
parsarg_initialize(
    parsarg_t* pa,
    int        argc,
    char**     argv
);

void
parsarg_deinitialize(
    parsarg_t* pa
);

char*  /* NULLABLE */
parsarg_pop_name(
    parsarg_t* pa,
    size_t*    len
);

bool
parsarg_pop_value(
    parsarg_t* pa,
    char**     str
);

bool
parsarg_finished(
    const parsarg_t* pa
);
