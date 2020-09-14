#include "./program.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "util/math/algorithm.h"

#include "./shader.h"

gleasy_program_t gleasy_program_new(
    const char* header, size_t header_len,
    const char* vsrc,   size_t vsrc_len,
    const char* fsrc,   size_t fsrc_len) {
  assert(header != NULL || header_len == 0);
  assert(vsrc   != NULL || vsrc_len   == 0);
  assert(fsrc   != NULL || fsrc_len   == 0);

  gleasy_program_t program = glCreateProgram();
  if (program == 0) {
    fprintf(stderr, "failed to create program");
    abort();
  }

  const gleasy_shader_t vshader =
      gleasy_shader_new(GL_VERTEX_SHADER, header, header_len, vsrc, vsrc_len);
  const gleasy_shader_t fshader =
      gleasy_shader_new(GL_FRAGMENT_SHADER, header, header_len, fsrc, fsrc_len);

  glAttachShader(program, vshader);
  glDeleteShader(vshader);

  glAttachShader(program, fshader);
  glDeleteShader(fshader);

  glLinkProgram(program);

  GLint ok;
  glGetProgramiv(program, GL_LINK_STATUS, &ok);
  if (ok == GL_FALSE) {
    char log[1024];
    const int len =
        gleasy_program_get_log(program, log, sizeof(log)/sizeof(log[0]));
    fprintf(stderr, "failed to link program\n%.*s\n", len, log);
    abort();
  }
  return program;
}

size_t gleasy_program_get_log(
    gleasy_program_t program, char* dst, size_t maxlen) {
  assert(program != 0);

  GLint len;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
  if (dst == NULL) return len;

  len = MATH_MIN(len, (GLint) maxlen);
  if (len == 0) return 0;

  glGetProgramInfoLog(program, len, &len, (GLchar*) dst);
  return len;
}
