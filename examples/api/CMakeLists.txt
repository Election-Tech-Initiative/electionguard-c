cmake_minimum_required(VERSION 3.13)

project("ElectionGuard API Example Client" VERSION 0.0.1 LANGUAGES C)

enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}/../../cmake
    ${CMAKE_MODULE_PATH}
)

add_executable(api
    ${PROJECT_SOURCE_DIR}/main.c
)

find_package(ElectionGuard REQUIRED)

find_package(GMP REQUIRED)
message(STATUS "GMP library found.")
message(STATUS "GMP include dir is: ${GMP_INCLUDE_DIR}")
message(STATUS "GMP library is: ${GMP_LIBRARY}")

if(APPLE)
    target_link_libraries(api electionguard ${GMP_LIBRARY})
else()
    target_link_libraries(api electionguard gmp)
endif()

if (WIN32)
    target_link_libraries(api Bcrypt)
endif (WIN32)

add_test(NAME api COMMAND api)

install(TARGETS api)
