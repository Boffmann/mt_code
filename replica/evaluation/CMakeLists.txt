set(target evaluator)

set(include_path 
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")

set(sources
  ${source_path}/evaluator.c
)

#
# Create executable
#

# Build executable
add_library(${target} SHARED
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