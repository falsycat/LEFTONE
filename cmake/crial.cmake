function(target_crial_sources target)
  set(crial ${LEFTONE_TOOL_DIR}/crial.sh)

  foreach (path ${ARGN})
    get_filename_component(dirname  ${path} DIRECTORY)
    get_filename_component(basename ${path} NAME_WE)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/crial/${dirname})

    set(in  ${CMAKE_CURRENT_SOURCE_DIR}/${path})
    set(out ${CMAKE_CURRENT_BINARY_DIR}/crial/${dirname}/${basename}.h)
    add_custom_command(
        OUTPUT  ${out}
        COMMAND bash ${crial} < ${in} > ${out}
        DEPENDS ${path} ${crial}
        COMMENT "generating serializer from ${path}"
        VERBATIM)
    target_sources(${target} PRIVATE ${out})
  endforeach()
endfunction()
