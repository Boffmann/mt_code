# LISTENER CMakeLists.txt

#
# External dependencies
#

include_directories($ENV{OSPL_HOME}/include/dcps/C/SAC)

LINK_DIRECTORIES($ENV{OSPL_HOME}/lib)

set(target worker)

set(include_path 
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/dds_lib/include"
)
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(generated_path "${CMAKE_BINARY_DIR}/gen")

set(dds_lib revpi_dds)

set(sources
  ${source_path}/main.c
)

#
# Create executable
#

# Build executable
add_executable(${target}
  ${sources}
)

# add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

#
# Include directories
#

target_include_directories(${target}
  PRIVATE
  ${include_path}
  ${generated_path}
  # ${CMAKE_SOURCE_DIR}/src/dds_lib/include
)

#
# Libraries
#

target_link_libraries(${target}
  PRIVATE
  ${dds_lib}
)

add_subdirectory(datamodel)
add_subdirectory(dds_lib)