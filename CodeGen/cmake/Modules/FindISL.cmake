# FindISL.cmake

# Look for the ISL header files
find_path(ISL_INCLUDE_DIR
  NAMES isl/ctx.h
  PATHS /usr/include /usr/local/include /usr/lib/gcc/x86_64-redhat-linux/12/
)

# Look for the ISL library
find_library(ISL_LIBRARY
  NAMES isl
  PATHS /usr/lib/gcc/x86_64-redhat-linux/12/
)

# Handle the standard arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ISL DEFAULT_MSG ISL_LIBRARY ISL_INCLUDE_DIR)

# Mark the variables as advanced (optional)
mark_as_advanced(ISL_INCLUDE_DIR ISL_LIBRARY)
