cmake_minimum_required(VERSION 3.10)
project(POLYHEDRON)

# Find LLVM and Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Found Clang ${CLANG_PACKAGE_VERSION}")
message(STATUS "Using ClangConfig.cmake in: ${CLANG_DIR}")

# Include the necessary directories
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# all cpp files in src/
file(GLOB SOURCES "src/*.cpp")

# Add executable
add_executable(polyhedron ${SOURCES})

# Map the LLVM components to the libraries
llvm_map_components_to_libnames(LLVM_LIBS support core irreader)

target_compile_options(polyhedron PRIVATE -fno-rtti)

# Link the executable with Clang and LLVM libraries
target_link_libraries(polyhedron
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  ${LLVM_LIBS}  # LLVM libraries
)
# cmake_minimum_required(VERSION 3.10)
# project(POLYHEDRON)

# set(LLVM_LINK_COMPONENTS support)

# add_executable(polyhedron
#   src/main.cpp
#   )
# target_link_libraries(polyhedron
#   PRIVATE
#   clangAST
#   clangASTMatchers
#   clangBasic
#   clangFrontend
#   clangSerialization
#   clangTooling
#   )


# find_package(LLVM REQUIRED CONFIG)
# find_package(Clang REQUIRED CONFIG)

# message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
# message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
# message(STATUS "Found Clang ${CLANG_PACKAGE_VERSION}")
# message(STATUS "Using ClangConfig.cmake in: ${CLANG_DIR}")

# include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
# add_definitions(${LLVM_DEFINITIONS})

# # Capture the llvm-config output for linking
# execute_process(
#     COMMAND llvm-config --ldflags
#     OUTPUT_VARIABLE LLVM_LDFLAGS
#     OUTPUT_STRIP_TRAILING_WHITESPACE
# )

# execute_process(
#     COMMAND llvm-config --libs
#     OUTPUT_VARIABLE LLVM_LIBS
#     OUTPUT_STRIP_TRAILING_WHITESPACE
# )

# message(STATUS "LLVM Linker Flags: ${LLVM_LDFLAGS}")
# message(STATUS "LLVM Libraries: ${LLVM_LIBS}")

# add_executable(POLYHEDRON src/main.cpp)

# target_link_libraries(POLYHEDRON
#     PRIVATE
#     clangTooling
#     clangBasic
#     clangASTMatchers
#     clangFrontend
#     clangSerialization
#     clangDriver
#     clangParse
#     clangSema
#     clangAnalysis
#     clangEdit
#     clangAST
#     clangLex
#     clangRewriteFrontend
#     clangRewrite
#     clangCrossTU
#     clangIndex
#     clangToolingCore
#     ${LLVM_LIBS}  # Use the output of llvm-config --libs
# )

# # Add the LLVM linker flags directly to the target link libraries
# target_link_options(POLYHEDRON PRIVATE ${LLVM_LDFLAGS})
#
