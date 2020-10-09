add_custom_target(benum-generated)

function(target_benum_sources target)
  set(benum ${LEFTONE_TOOL_DIR}/benum.sh)

  set(output_files "")
  foreach (path ${ARGN})
    get_filename_component(dirname  ${path} DIRECTORY)
    get_filename_component(basename ${path} NAME_WE)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/benum/${dirname})

    set(in  ${CMAKE_CURRENT_SOURCE_DIR}/${path})
    set(out ${CMAKE_CURRENT_BINARY_DIR}/benum/${dirname}/${basename})
    add_custom_command(
        OUTPUT  ${out}.c ${out}.h
        COMMAND bash ${benum} ${out} ${in} < ${in}
        DEPENDS ${path} ${benum}
        COMMENT "generating benum utilities for ${path}"
        VERBATIM)
    target_sources(${target} PRIVATE ${out}.c)

    list(APPEND output_files ${out}.c ${out}.h)
  endforeach()

  add_custom_target(${target}-benum-generated DEPENDS ${output_files})
  add_dependencies(${target} ${target}-benum-generated)
  add_dependencies(benum-generated ${target}-benum-generated)

endfunction()
