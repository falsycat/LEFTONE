#pragma once

#include <stddef.h>

#include <GL/glew.h>

typedef GLuint gleasy_shader_t;

gleasy_shader_t  /* OWNERSHIP */
gleasy_shader_new(
    GLenum      type,
    const char* header,
    size_t      header_len,
    const char* src,
    size_t      src_len
);

size_t
gleasy_shader_get_log(
    gleasy_shader_t shader,
    char*           dst,  /* when NULL, returns actual size */
    size_t          maxlen
);
