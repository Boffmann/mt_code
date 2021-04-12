# LISTENER CMakeLists.txt

#
# External dependencies
#

include_directories($ENV{OSPL_HOME}/include)
include_directories($ENV{OSPL_HOME}/include/sys)
include_directories($ENV{OSPL_HOME}/include/dcps/C/SAC)
include_directories($ENV{OSPL_HOME}/include/dcps/C/C99)

LINK_DIRECTORIES($ENV{OSPL_HOME}/lib)

set(OSPL_LINK_LIBS PUBLIC ddskernel dcpssac)

set(target RevPiTrainSimulator)

set(include_path 
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../jsmn"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../replica/state"
)
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(generated_path "${CMAKE_BINARY_DIR}/gen")

set(datamodel_lib datamodel)

set(sources
  ${source_path}/CheckStatus.c
  ${source_path}/DDSEntitiesCreator.c
  ${source_path}/main.c
  ${source_path}/scenario.c
  ${source_path}/simulator.c
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
)

#
# Libraries
#

find_library(State_lib state HINTS ../../replica/build)

target_link_libraries(${target}
  PRIVATE
  ${OSPL_LINK_LIBS}
  ${State_lib}
  ${datamodel_lib}
)