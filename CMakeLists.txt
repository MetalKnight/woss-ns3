set(NS3_WITH_WOSS_SOURCE
    ""
    CACHE
      PATH
      "Path to WOSS source directory, for NS-3 WOSS Integration Framework support"
)
set(NS3_WITH_WOSS_LIBRARY
    ""
    CACHE
      PATH
      "Path to WOSS library directory, for NS-3 WOSS Integration Framework support"
)

set(NS3_WITH_NETCDF_INSTALL
    ""
    CACHE
      PATH
      "Path to NetCDF legacy library install, for WOSS Integration Framework support"
)
set(NS3_WITH_NETCDF4_INSTALL
    ""
    CACHE
      PATH
      "Path to NetCDF4 and HDF5 library install, for WOSS Integration Framework support"
)

set(ENABLE_WOSS
    "OFF"
    CACHE INTERNAL
          "ON if WOSS can be built"
)

find_external_library(
  DEPENDENCY_NAME WOSS
  LIBRARY_NAME WOSS
  HEADER_NAMES
    location-definitions.h
    woss-manager.h
    woss-db-manager.h
  SEARCH_PATHS ${NS3_WITH_WOSS_SOURCE}
               ${NS3_WITH_WOSS_LIBRARY}
  PATH_SUFFIXES
    /woss
    /woss_def
    /woss_db
    /woss/woss_def
    /woss/woss_db
    /build
    /build/lib
)

set(woss_libraries)

# If we can't find WOSS sources or library, don't process this module
if(NOT
   ${WOSS_FOUND}
)
  message(
    ${HIGHLIGHTED_STATUS}
    "Skipping contrib/woss-ns3"
  )
  return()
else()
  message(STATUS "WOSS library was found: ${WOSS_LIBRARIES}")
  set(ENABLE_WOSS
      "ON"
      CACHE INTERNAL
            "ON if woss-ns3 can be built"
  )
  include_directories(${WOSS_INCLUDE_DIRS})
  set(woss_libraries
      ${WOSS_LIBRARIES}
  )
endif()

# Process optional NETCDF libraries
find_external_library(
  DEPENDENCY_NAME NETCDF
  HEADER_NAMES
    netcdf.h
    ncFile.h
  LIBRARY_NAME netcdf_c++
               netcdf
  SEARCH_PATHS ${NS3_WITH_NETCDF_INSTALL}
  PATH_SUFFIXES
    /include
    /lib
)

find_external_library(
  DEPENDENCY_NAME NETCDF4
  HEADER_NAMES
    hdf5.h
    netcdf.h
    ncFile.h
  LIBRARY_NAMES
    netcdf_c++4
    netcdf
    hdf5
  SEARCH_PATHS ${NS3_WITH_NETCDF4_INSTALL}
  PATH_SUFFIXES
    /include
    /lib
)

set(netcdf_libraries)
if(${NETCDF4_FOUND})
  message(STATUS "NETCDF4 was found.")
  add_definitions(
    -DWOSS_NETCDF_SUPPORT
    -DWOSS_NETCDF4_SUPPORT
  )
  include_directories(${NETCDF4_INCLUDE_DIRS})
  set(netcdf_libraries
      ${NETCDF4_LIBRARIES}
  )
else()
  if(${NETCDF_FOUND})
    message(STATUS "NETCDF was found.")
    add_definitions(-DWOSS_NETCDF_SUPPORT)
    include_directories(${NETCDF_INCLUDE_DIRS})
    set(netcdf_libraries
        ${NETCDF_LIBRARIES}
    )
  else()
    message(STATUS "NETCDF was not found")
  endif()
endif()

add_definitions(
  -DNS3_WOSS_SUPPORT
  -DWOSS_MULTITHREAD
)

build_lib(
  LIBNAME woss-ns3
  SOURCE_FILES
    model/definitions/woss-location.cc
    model/definitions/woss-time-reference.cc
    model/definitions/woss-random-generator.cc
    model/woss-prop-model.cc
    model/woss-channel.cc
    model/woss-position-allocator.cc
    model/woss-waypoint-mobility-model.cc
    helper/woss-helper.cc
  HEADER_FILES
    model/definitions/woss-location.h
    model/definitions/woss-time-reference.h
    model/definitions/woss-random-generator.h
    model/woss-prop-model.h
    model/woss-channel.h
    model/woss-position-allocator.h
    model/woss-waypoint-mobility-model.h
    helper/woss-helper.h
  LIBRARIES_TO_LINK
    ${libnetanim}
    ${libnetwork}
    ${libenergy}
    ${libmobility}
    ${libuan}
    ${woss_libraries}
    ${netcdf_libraries}
  TEST_SOURCES test/woss-test.cc
)

