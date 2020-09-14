#pragma once

#include <stddef.h>

#include <GL/glew.h>

typedef GLuint gleasy_program_t;

gleasy_program_t  /* OWNERSHIP */
gleasy_program_new(
    const char* header,
    size_t      header_len,
    const char* vsrc,
    size_t      vsrc_len,
    const char* fsrc,
    size_t      fsrc_len
);

size_t
gleasy_program_get_log(
    gleasy_program_t program,
    char*            dst,  /* when NULL, returns actual size */
    size_t           maxlen
);
