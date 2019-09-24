cmake_minimum_required(VERSION 3.13)

project("ElectionGuard SDK" LANGUAGES C)

enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(electionguard STATIC
    ${PROJECT_SOURCE_DIR}/src/voting/coordinator.c
    ${PROJECT_SOURCE_DIR}/src/keyceremony/coordinator.c
    ${PROJECT_SOURCE_DIR}/src/keyceremony/trustee.c
    ${PROJECT_SOURCE_DIR}/src/voting/nouns.c
    ${PROJECT_SOURCE_DIR}/src/voting/tracker.c
    ${PROJECT_SOURCE_DIR}/src/voting/encrypter.c
    ${PROJECT_SOURCE_DIR}/src/decryption/coordinator.c
    ${PROJECT_SOURCE_DIR}/src/decryption/trustee.c
    ${PROJECT_SOURCE_DIR}/src/serialize/builtins.c
    ${PROJECT_SOURCE_DIR}/src/serialize/state.c
    ${PROJECT_SOURCE_DIR}/src/serialize/decryption.c
    ${PROJECT_SOURCE_DIR}/src/serialize/trustee_state.c
    ${PROJECT_SOURCE_DIR}/src/serialize/voting.c
    ${PROJECT_SOURCE_DIR}/src/serialize/keyceremony.c
    ${PROJECT_SOURCE_DIR}/src/serialize/crypto.c
    ${PROJECT_SOURCE_DIR}/src/crypto.c)

target_include_directories(electionguard
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(electionguard PRIVATE -Wall -Wextra -pedantic -pedantic-errors -Wunreachable-code -Wmissing-field-initializers)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(electionguard PRIVATE -Wgnu-empty-initializer)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(electionguard PRIVATE -Wenum-compare)
endif()

# examples
add_subdirectory(examples)

# documentation
set(DOXYGEN_INPUT ${PROJECT_SOURCE_DIR}/Doxyfile)
set(DOCS_OUTPUT ${PROJECT_BINARY_DIR}/html)
add_custom_target(docs
    COMMAND doxygen ${DOXYGEN_INPUT}
    COMMAND sphinx-build -b html ${PROJECT_SOURCE_DIR}/doc ${DOCS_OUTPUT}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
