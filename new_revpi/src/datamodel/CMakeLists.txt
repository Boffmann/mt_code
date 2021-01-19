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
  ${source_path}/datamodel.idl
)

set (sources
    ${generated_path}/datamodel.h
    ${generated_path}/datamodelDcps.h
    ${generated_path}/datamodelSacDcps.h
    ${generated_path}/datamodelSplDcps.h
    ${generated_path}/datamodelSacDcps.c
    ${generated_path}/datamodelSplDcps.c
)

#
# Create library
#

# Generate idl files
execute_process(COMMAND $ENV{OSPL_HOME}/bin/idlpp -S -l c -d ${generated_path} ${idl_file})

# Make generated object files position independent in order for them to be linkable to a shared lib
add_compile_options(-fPIC)

# Build library
add_library( ${target} STATIC
  ${sources}
)

target_link_libraries(${target}
  PUBLIC
  ${OSPL_LINK_LIBS}
)
