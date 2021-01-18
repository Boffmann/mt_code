cmake_minimum_required(VERSION 3.1)


set(META_PROJECT_NAME        "RevPiRedundancy")
set(META_PROJECT_DESCRIPTION "RevPiRedundancy")
set(META_AUTHOR_ORGANIZATION "Operating System and Middleware Group")
set(META_AUTHOR_DOMAIN       "TODO")
set(META_AUTHOR_MAINTAINER   "hendrik.tjabben@student.hpi.de")
set(META_VERSION_MAJOR       "1")
set(META_VERSION_MINOR       "0")
set(META_VERSION_PATCH       "0")
set(META_VERSION_REVISION    "${GIT_REV}")
set(META_VERSION             "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}")
set(META_NAME_VERSION        "${META_PROJECT_NAME} v${META_VERSION} (${META_VERSION_REVISION})")
set(META_CMAKE_INIT_SHA      "${GIT_REV}")

string(MAKE_C_IDENTIFIER ${META_PROJECT_NAME} META_PROJECT_ID)
string(TOUPPER ${META_PROJECT_ID} META_PROJECT_ID)

#
# Project configuration options
#

# Project options
option(BUILD_SHARED_LIBS      "Build shared instead of static libraries."              ON)
add_compile_options(-Wall -Wextra -pedantic -Werror)

#
# Declare project
#
project(${META_PROJECT_NAME} C)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# Create version file
file(WRITE "${PROJECT_BINARY_DIR}/VERSION" "${META_NAME_VERSION}")

add_subdirectory(src)
