#include "./shader.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "util/math/algorithm.h"

gleasy_shader_t gleasy_shader_new(
    GLenum type,
    const char* header, size_t header_len,
    const char* src,    size_t src_len) {
  assert(header != NULL || header_len == 0);
  assert(src    != NULL || src_len    == 0);

  const GLuint shader = glCreateShader(type);
  if (shader == 0) {
    fprintf(stderr, "failed to create shader\n");
    abort();
  }

  const GLchar* srcs[] = { header,     src, };
  const GLint   lens[] = { header_len, src_len, };
  const size_t  offset = (header_len == 0? 1: 0);
  glShaderSource(shader, 2-offset, srcs+offset, lens+offset);
  glCompileShader(shader);

  GLint ok;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (ok == GL_FALSE) {
    char log[1024];
    const int loglen =
        gleasy_shader_get_log(shader, log, sizeof(log)/sizeof(log[0]));
    fprintf(stderr, "failed to compile shader\n%.*s\n", loglen, log);
    abort();
  }
  return shader;
}

size_t gleasy_shader_get_log(
    gleasy_shader_t shader, char* dst, size_t maxlen) {
  assert(shader != 0);

  GLint len;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
  if (dst == NULL) return len;

  len = MATH_MIN(len, (GLint) maxlen);
  if (len == 0) return 0;

  glGetShaderInfoLog(shader, len, &len, (GLchar*) dst);
  return len;
}
