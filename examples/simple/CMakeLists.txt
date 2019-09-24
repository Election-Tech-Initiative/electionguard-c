cmake_minimum_required(VERSION 3.13)

project("ElectionGuard Simple Example Client" VERSION 0.0.1 LANGUAGES C)

enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

add_executable(simple
    ${PROJECT_SOURCE_DIR}/main_params.h
    ${PROJECT_SOURCE_DIR}/main_voting.h
    ${PROJECT_SOURCE_DIR}/main_decryption.c
    ${PROJECT_SOURCE_DIR}/main_keyceremony.c
    ${PROJECT_SOURCE_DIR}/main.c
    ${PROJECT_SOURCE_DIR}/main_voting.c
    ${PROJECT_SOURCE_DIR}/main_keyceremony.h
    ${PROJECT_SOURCE_DIR}/main_decryption.h
)

find_package(ElectionGuard REQUIRED)
target_link_libraries(simple electionguard)

add_test(NAME simple COMMAND simple)

install(TARGETS simple)
