project(miniaudio C)

set_source_files_properties(repo/miniaudio.h
  DIRECTORY .
  PROPERTIES
    LANGUAGE C
    COMPILE_FLAGS "-x c"
)
add_library(miniaudio
  repo/miniaudio.h
)
target_compile_definitions(miniaudio
  PRIVATE
    MINIAUDIO_IMPLEMENTATION
  PUBLIC
    MA_NO_ENCODING
    MA_NO_FLAC
    $<$<CONFIG:Debug>:MA_DEBUG_OUTPUT>
)
target_include_directories(miniaudio
    SYSTEM INTERFACE repo
)
target_link_libraries(miniaudio
  $<$<PLATFORM_ID:Linux>:dl>
  $<$<PLATFORM_ID:Linux>:m>
  $<$<PLATFORM_ID:Linux>:pthread>
)
