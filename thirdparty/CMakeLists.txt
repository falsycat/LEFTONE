set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(BUILD_TESTING OFF)
set(TINYCTHREAD_DISABLE_TESTS ON)

add_subdirectory(miniaudio)
add_subdirectory(tinycthread)
