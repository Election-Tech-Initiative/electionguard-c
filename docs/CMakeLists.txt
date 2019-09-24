option(BUILD_DOCUMENTATION
    "Create and install the HTML based API documentation (requires Doxygen and Shinx)" OFF)

IF(BUILD_DOCUMENTATION)
  find_package(Doxygen REQUIRED)
  find_package(Sphinx REQUIRED)

  set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/include)
  set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
  set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
  set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  get_target_property(ELECTIONGUARD_PUBLIC_HEADER_DIR electionguard INTERFACE_INCLUDE_DIRECTORIES)
  file(GLOB_RECURSE ELECTIONGUARD_PUBLIC_HEADERS ${ELECTIONGUARD_PUBLIC_HEADER_DIR}/*.h)

  add_custom_command(
      OUTPUT ${DOXYGEN_INDEX_FILE}
      DEPENDS ${ELECTIONGUARD_PUBLIC_HEADERS}
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
      MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
      COMMENT "Generating doxygen XML"
  )

  add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

  set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
  set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/html)
  set(SPHINX_CACHE ${CMAKE_CURRENT_BINARY_DIR}/_doctrees)
  set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

  set(ELECTIONGUARD_RST
      ${PROJECT_SOURCE_DIR}/README.rst
      ${PROJECT_SOURCE_DIR}/include/electionguard/README.rst
      ${PROJECT_SOURCE_DIR}/include/electionguard/keyceremony/README.rst
      ${PROJECT_SOURCE_DIR}/include/electionguard/voting/README.rst
      ${PROJECT_SOURCE_DIR}/include/electionguard/decryption/README.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/index.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/examples.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/voting.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/keyceremony.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/examples/simple.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/index.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/crypto.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/trustee_state.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/voting/coordinator.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/voting/encrypter.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/voting/messages.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/voting/tracker.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/decryption/coordinator.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/decryption/trustee.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/decryption/messages.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/keyceremony/coordinator.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/keyceremony/trustee.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/keyceremony/messages.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/api/max_values.rst
      ${CMAKE_CURRENT_SOURCE_DIR}/decryption.rst
  )

  add_custom_command(
    OUTPUT ${SPHINX_INDEX_FILE}
    COMMAND ${SPHINX_EXECUTABLE} -b html
      -Dbreathe_projects.ElectionGuardSDK=${DOXYGEN_OUTPUT_DIR}/xml
      -d ${SPHINX_CACHE}
      ${SPHINX_SOURCE} ${SPHINX_BUILD}
      WORKING_DIRECTORY ${SPHINX_SOURCE}
    MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
    DEPENDS ${ELECTIONGUARD_RST} ${DOXYGEN_INDEX_FILE}
    COMMENT "Generating HTML documentation"
  )

  add_custom_target(Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})

  install(DIRECTORY ${SPHINX_BUILD} TYPE DOC)
endif()
