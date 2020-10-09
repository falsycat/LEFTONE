function(target_any_sources target)
  set(bin2c ${LEFTONE_TOOL_DIR}/bin2c.sh)

  foreach (path ${ARGN})
    get_filename_component(dirname ${path} DIRECTORY)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/anysrc/${dirname})

    set(name ${target}_${path}_)
    set(in  ${CMAKE_CURRENT_SOURCE_DIR}/${path})
    set(out ${CMAKE_CURRENT_BINARY_DIR}/anysrc/${path})
    add_custom_command(
        OUTPUT  ${out}.c ${out}.h
        COMMAND cat ${in} | bash ${bin2c} ${name} ${out}
        DEPENDS ${path} ${bin2c}
        COMMENT "converting ${path} to C header"
        VERBATIM)
    target_sources(${target} PRIVATE ${out}.c ${out}.h)
  endforeach()
endfunction()
