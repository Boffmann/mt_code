# DATAMODEL CMakeLists.txt

#
# External dependencies
#

include_directories($ENV{OSPL_HOME}/include/sys)
include_directories($ENV{OSPL_HOME}/include/dcps/C/SAC)
include_directories($ENV{OSPL_HOME}/include/dcps/C/C99)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

LINK_DIRECTORIES($ENV{OSPL_HOME}/lib)

set(OSPL_LINK_LIBS PUBLIC ddskernel dcpsc99 dcpssac)

#
# Library name and options
#

# Target name
set(target datamodel)

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(generated_path "${CMAKE_BINARY_DIR}/gen")

set(idl_file
  ${source_path}/listener_task_data.idl
)

set (sources
    ${generated_path}/listener_task_data.h
    ${generated_path}/listener_task_dataDcps.h
    ${generated_path}/listener_task_dataSacDcps.h
    ${generated_path}/listener_task_dataSplDcps.h
    ${generated_path}/listener_task_dataSacDcps.c
    ${generated_path}/listener_task_dataSplDcps.c
)

#
# Create library
#

# Generate idl files
execute_process(COMMAND $ENV{OSPL_HOME}/bin/idlpp -S -l c -d ${generated_path} ${idl_file})

# Build library
add_library( ${target} SHARED
  ${sources}
)

target_link_libraries(${target}
  PUBLIC
  ${OSPL_LINK_LIBS}
)
