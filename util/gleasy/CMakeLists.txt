add_library(gleasy
  atlas.c
  framebuffer.c
  program.c
  shader.c
)
target_link_libraries(gleasy
  GLEW::GLEW
  OpenGL::GL

  container
  math
  memory
)
