find_program(SPHINX_EXECUTABLE 
    NAMES sphinx-build
    DOC "Sphinx documentation generator"
)
 
include(FindPackageHandleStandardArgs)
 
find_package_handle_standard_args(Sphinx DEFAULT_MSG SPHINX_EXECUTABLE)
