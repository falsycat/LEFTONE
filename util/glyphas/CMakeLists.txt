add_library(glyphas
  aligner.c
  block.c
  cache.c
  context.c
  drawer.c
  face.c
  glyph.c
)
target_any_sources(glyphas
  drawer.vshader
  drawer.fshader
)
target_link_libraries(glyphas
  Freetype::Freetype
  GLEW::GLEW
  OpenGL::GL

  chaos
  container
  conv
  gleasy
  math
  memory
)

if (BUILD_TESTING)
  add_executable(glyphas-test test.c)
  target_link_libraries(glyphas-test
    ${SDL2_TARGET}

    glyphas)
endif()
