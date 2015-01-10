cmake_minimum_required(VERSION 2.8)
project(flam3boinc)

find_package(LibXml2 REQUIRED)
if(WIN32)
    set(PNG_NAMES libpng12_static)
else()
    set(PNG_NAMES png12)
endif()
find_package(PNG REQUIRED)
find_package(Threads REQUIRED)

add_library(flam3 STATIC
    flam3-3.0.1/src/flam3.c
    flam3-3.0.1/src/filters.c
    flam3-3.0.1/src/parser.c
    flam3-3.0.1/src/variations.c
    flam3-3.0.1/src/interpolation.c
    flam3-3.0.1/src/palettes.c
    #flam3-3.0.1/src/jpeg.c
    flam3-3.0.1/src/png.c
    flam3-3.0.1/src/isaac.c
)
target_compile_definitions(flam3 PRIVATE SVN_REV="exported" PACKAGE_DATA_DIR="/nonexistent")
target_include_directories(flam3 PUBLIC flam3-3.0.1/src ${LIBXML2_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})
target_link_libraries(flam3 PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${LIBXML2_LIBRARIES} ${PNG_LIBRARIES})


find_path(BOINC_INCLUDE_DIR boinc_api.h PATH_SUFFIXES boinc)

find_library(BOINCAPI_LIBRARY  boinc_api   PATH_SUFFIXES api)
find_library(BOINCLIB_LIBRARY  boinc       PATH_SUFFIXES lib)

add_executable(flam3boinc main.cpp)
target_include_directories(flam3boinc PUBLIC ${BOINC_INCLUDE_DIR})
target_link_libraries(flam3boinc flam3 ${BOINCAPI_LIBRARY} ${BOINCLIB_LIBRARY})

if(WIN32)
    # winsock needed by libxml2. Ideally this should come from
    # libxml2's cmake files...
    target_link_libraries(flam3boinc ws2_32)
endif()

if(NOT MSVC)
    target_compile_options(flam3boinc PUBLIC -std=c++11)
endif()
