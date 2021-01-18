# WORKER CMakeLists.txt

#
# External dependencies
#

include_directories($ENV{OSPL_HOME}/include/dcps/C/SAC)

# OSPL libs
LINK_DIRECTORIES($ENV{OSPL_HOME}/lib)

set(target revpi_worker)

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")

set(dds_lib revpi_dds)

set(sources
    ${source_path}/main.c
    ${source_path}/DIO.c
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
    ${include_path}
    ${CMAKE_SOURCE_DIR}/src
)

#
# Libraries
#

target_link_libraries(${target}
    PRIVATE
    ${dds_lib}
)