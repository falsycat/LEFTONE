function(target_source_of_source target)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  foreach (path ${ARGN})
    get_filename_component(dirname  ${path} DIRECTORY)
    get_filename_component(basename ${path} NAME_WE)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sos/${dirname})

    set(sos_target ${target}-${basename}-sos-generator)
    set(in         ${CMAKE_CURRENT_SOURCE_DIR}/${path})
    set(out        ${CMAKE_CURRENT_BINARY_DIR}/sos/${path})

    add_executable(${sos_target} ${in})
    add_custom_command(
        OUTPUT  ${out}
        COMMAND ${sos_target} > ${out}
        DEPENDS ${sos_target}
        COMMENT "generating ${path}"
        VERBATIM)
    target_sources(${target} PRIVATE ${out})
  endforeach()
endfunction()
