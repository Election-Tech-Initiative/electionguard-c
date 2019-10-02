cmake_minimum_required(VERSION 3.13)

project("ElectionGuard Simple Example Client" VERSION 0.0.1 LANGUAGES C)

enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}/../../cmake
    ${CMAKE_MODULE_PATH}
)

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

find_package(GMP REQUIRED)
message(STATUS "GMP library found.")
message(STATUS "GMP include dir is: ${GMP_INCLUDE_DIR}")
message(STATUS "GMP library is: ${GMP_LIBRARY}")

target_link_libraries(simple electionguard gmp)

if (WIN32)
    target_link_libraries(simple Bcrypt)
endif (WIN32)

add_test(NAME simple COMMAND simple)

install(TARGETS simple)
