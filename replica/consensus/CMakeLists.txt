# LISTENER CMakeLists.txt

#
# External dependencies
#

include_directories($ENV{OSPL_HOME}/include)
include_directories($ENV{OSPL_HOME}/include/sys)
include_directories($ENV{OSPL_HOME}/include/dcps/C/SAC)
include_directories($ENV{OSPL_HOME}/include/dcps/C/C99)

include_directories($ENV{EVENTLIB}/include)
include_directories($ENV{EVENTLIB}/build/include)

LINK_DIRECTORIES($ENV{OSPL_HOME}/lib)
LINK_DIRECTORIES($ENV{EVENTLIB}/build/lib)

set(OSPL_LINK_LIBS PUBLIC ddskernel dcpssac)
set(EVENT_LIBS PUBLIC event)

set(target consensus)

set(include_path 
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}"
)
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(generated_path "${CMAKE_BINARY_DIR}/gen")

set(datamodel_lib datamodel)
set(DDSCreator_lib DDSCreator)
set(DIO_lib DIO)

set(sources
  ${source_path}/DDSConsensusManager.c
  ${source_path}/leaderElection.c
  ${source_path}/replica.c
  ${source_path}/spare.c
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
  ${generated_path}
)

#
# Libraries
#

target_link_libraries(${target}
  PRIVATE
  ${OSPL_LINK_LIBS}
  ${EVENT_LIBS}
  pthread
  ${datamodel_lib}
  ${DDSCreator_lib}
  ${DIO_lib}
)