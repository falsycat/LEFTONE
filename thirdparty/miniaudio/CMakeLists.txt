add_library(miniaudio
  miniaudio.c
)
target_link_libraries(miniaudio
  $<$<PLATFORM_ID:Linux>:dl>
  $<$<PLATFORM_ID:Linux>:m>
  $<$<PLATFORM_ID:Linux>:pthread>
)
