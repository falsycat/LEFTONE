add_executable(app-sdl
  args.c
  event.c
  main.c
)
target_link_libraries(app-sdl
  GLEW::GLEW
  ${SDL2_TARGET}

  conv
  parsarg

  loscene
)
