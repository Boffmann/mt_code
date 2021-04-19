include_directories($ENV{OSPL_HOME}/include)
include_directories($ENV{OSPL_HOME}/include/sys)
include_directories($ENV{OSPL_HOME}/include/dcps/C/SAC)
include_directories($ENV{OSPL_HOME}/include/dcps/C/C99)

LINK_DIRECTORIES($ENV{OSPL_HOME}/lib)

set(OSPL_LINK_LIBS PUBLIC ddskernel dcpssac)

set(target evaluator)
set(datamodel_lib datamodel)

set(include_path 
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(State_lib state)

set(sources
  ${source_path}/evaluator.c
)
set(generated_path "${CMAKE_BINARY_DIR}/gen")

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
  ${generated_path}
)

target_link_libraries(${target}
  PRIVATE
  ${OSPL_LINK_LIBS}
  ${datamodel_lib}
  pthread
  ${State_lib}
)