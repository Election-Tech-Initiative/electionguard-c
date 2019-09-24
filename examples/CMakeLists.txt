# simple example
add_executable(simple
    ${CMAKE_CURRENT_SOURCE_DIR}/simple/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/simple/main_keyceremony.c
    ${CMAKE_CURRENT_SOURCE_DIR}/simple/main_voting.c
    ${CMAKE_CURRENT_SOURCE_DIR}/simple/main_decryption.c)
target_include_directories(simple PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(simple electionguard)

add_test(NAME simple COMMAND simple)

add_test(NAME build_simple
  COMMAND "${CMAKE_COMMAND}"
    --build "${CMAKE_BINARY_DIR}"
    --target simple
)

set_tests_properties(simple PROPERTIES FIXTURES_REQUIRED simple)
set_tests_properties(build_simple PROPERTIES FIXTURES_SETUP simple)
