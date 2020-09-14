function(target_source_of_source target)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sos)
  foreach (file ${ARGN})
    get_filename_component(name ${file} NAME_WE)

    set(sos_target sos-${target}-${name})
    set(in         ${CMAKE_CURRENT_SOURCE_DIR}/${file})
    set(out        ${CMAKE_CURRENT_BINARY_DIR}/sos/${file})

    add_executable(${sos_target} ${in})
    add_custom_command(
        OUTPUT  ${out}
        COMMAND ${sos_target} > ${out}
        DEPENDS ${sos_target}
        COMMENT "generating ${file}")
    target_sources(${target} PRIVATE ${out})
  endforeach()
endfunction()
