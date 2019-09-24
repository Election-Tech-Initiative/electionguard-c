cmake_minimum_required(VERSION 3.14)

project("ElectionGuard SDK" VERSION 0.0.1 LANGUAGES C)

enable_testing()

# Allow us to import cmake scripts from  ./cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(electionguard
    ${PROJECT_SOURCE_DIR}/src/electionguard/crypto_reps.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/voting/coordinator.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/voting/num_ballots.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/voting/message_reps.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/voting/nouns.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/voting/tracker.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/voting/nouns.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/voting/encrypter.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/builtins.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/trustee_state.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/state.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/keyceremony.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/decryption.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/voting.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/crypto.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/builtins.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/trustee_state.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/voting.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/decryption.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/keyceremony.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/state.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/serialize/crypto.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/decryption/coordinator.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/decryption/message_reps.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/decryption/trustee.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/keyceremony/coordinator.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/keyceremony/message_reps.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/keyceremony/trustee.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/uint4096.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/bignum.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/sha2-openbsd.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/sha2-openbsd.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/crypto.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/random_source.h
    ${PROJECT_SOURCE_DIR}/src/electionguard/random_source.c
    ${PROJECT_SOURCE_DIR}/src/electionguard/trustee_state_rep.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/max_values.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/trustee_state.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/voting/messages.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/voting/encrypter.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/voting/coordinator.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/voting/tracker.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/decryption/messages.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/decryption/coordinator.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/decryption/trustee.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/crypto.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/keyceremony/messages.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/keyceremony/coordinator.h
    ${PROJECT_SOURCE_DIR}/include/electionguard/keyceremony/trustee.h
)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(electionguard PUBLIC -Werror -Wall -Wextra -pedantic -pedantic-errors -Wunreachable-code -Wmissing-field-initializers -lgmp)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(electionguard PUBLIC -Werror --fsanitize=address -Wgnu-empty-initializer)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(electionguard PUBLIC -Werror -Wenum-compare)
endif()

# Set the public include directory depending on if the target is being exported
# or installed
target_include_directories(electionguard
    SYSTEM PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/electionguard
)

# Install the electionguard library in the default location, and associate
# electionguard with the ElectionGuard export
install(TARGETS electionguard EXPORT ElectionGuard)

# Install public header files
install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/electionguard
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h*"
)

# Generate the build-tree ElectionGuardConfig.cmake for use in other cmake
# projects without needing to install
export(
    EXPORT ElectionGuard
    FILE "${CMAKE_CURRENT_BINARY_DIR}/ElectionGuard/ElectionGuardConfig.cmake"
)

# Generate the install-tree ElectionGuardConfig.cmake for use in other cmake
# projects after this library has been installed
install(
    EXPORT ElectionGuard
    FILE ElectionGuardConfig.cmake
    DESTINATION lib/cmake/ElectionGuard
)

add_subdirectory(docs)

file(MAKE_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/simple_build")
add_test(NAME build_simple
   COMMAND "${CMAKE_COMMAND}"
      -DCMAKE_BUILD_TYPE=Deubg
      -E env ElectionGuard_DIR=${CMAKE_CURRENT_BINARY_DIR}/ElectionGuard
      "${CMAKE_CTEST_COMMAND}"
        --build-and-test "${PROJECT_SOURCE_DIR}/examples/simple" "${CMAKE_CURRENT_BINARY_DIR}/simple_build"
        --build-generator ${CMAKE_GENERATOR}
        --test-command simple
)

include(CheckIncludeFiles)
check_include_files(bcrypt.h HAVE_BCRYPTGENRANDOM)
configure_file(${PROJECT_SOURCE_DIR}/src/electionguard/random_source.h.in ${PROJECT_SOURCE_DIR}/src/electionguard/random_source.h)
