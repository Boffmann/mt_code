# WORKER CMakeLists.txt

#
# External dependencies
#

include_directories($ENV{OSPL_HOME}/include)
include_directories($ENV{OSPL_HOME}/include/sys)
include_directories($ENV{OSPL_HOME}/include/dcps/C/SAC)
include_directories($ENV{OSPL_HOME}/include/dcps/C/C99)

LINK_DIRECTORIES($ENV{OSPL_HOME}/lib)

set(OSPL_LINK_LIBS PUBLIC ddskernel dcpsc99)

set(target revpi_worker)

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")

set(dds_lib revpi_dds)

set(sources
    ${source_path}/main.c
)

#
# Create executable
#

# Build executable
add_executable(${target} ${sources})

# add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

#
# Include directories
#

target_include_directories(${target}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

#
# Libraries
#

target_link_libraries(${target}
    PRIVATE
    ${OSPL_LINK_LIBS}
    ${dds_lib}
)