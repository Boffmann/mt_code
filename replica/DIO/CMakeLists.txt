# LISTENER CMakeLists.txt

#
# External dependencies
#

set(target DIO)

set(include_path 
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")

set(sources
  ${source_path}/DIO.c
)

#
# Create library
#

# Make generated object files position independent in order for them to be linkable to a shared lib
add_compile_options(-fPIC)

# Build library
add_library(${target} STATIC
  ${sources}
)

# add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

#
# Include directories
#

target_include_directories(${target}
  PRIVATE
  ${include_path}
)