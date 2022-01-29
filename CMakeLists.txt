set(NS3_WITH_WOSS_SOURCE
    ""
    CACHE PATH
          "Path to WOSS source code, for WOSS Integration Framework support"
)
set(NS3_WITH_WOSS_LIBRARY
    ""
    CACHE PATH
          "Path to WOSS library, for NS-3 WOSS Integration Framework support"
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

if(NS3_WITH_WOSS_SOURCE)
  if(NOT
     (EXISTS
      ${NS3_WITH_WOSS_SOURCE})
  )
    message(STATUS "WOSS source was not found in ${NS3_WITH_WOSS_SOURCE}")
  else()
    message(STATUS "WOSS source was found")
    set(NS3_WOSS
        "ON"
        CACHE INTERNAL
              "ON if woss-ns3 can be built"
    )
  endif()
endif()

find_external_library(
  DEPENDENCY_NAME WOSS
  LIBRARY_NAME WOSS
  HEADER_NAMES location-definitions.h woss-manager.h woss-db-manager.h
  SEARCH_PATHS ${NS3_WITH_WOSS_LIBRARY}
  PATH_SUFFIXES /woss /woss_def /woss_db /woss/woss_def /woss/woss_db
)
message("${WOSS_INCLUDE_DIRS}")
if(NOT
   ${WOSS_FOUND}
)
  message(STATUS "WOSS library was not found")
  # If we can't find WOSS sources or library, don't process this module
  message(STATUS "Skipping contrib/woss-ns3")
  return()
else()
  message(STATUS "WOSS library was found: ${WOSS_LIBRARIES}")
  set(ENABLE_WOSS
      "ON"
      CACHE INTERNAL
            "ON if woss-ns3 can be built"
  )
endif()

find_external_library(
  DEPENDENCY_NAME NETCDF
  LIBRARY_NAME netcdf
  SEARCH_PATHS ${NS3_WITH_NETCDF4_INSTALL}
               ${NS3_WITH_NETCDF_INSTALL}
)

if(NOT
   ${NETCDF_FOUND}
)
  message(STATUS "NETCDF was not found")
else()
  message(STATUS "NETCDF was found: ${NETCDF_LIBRARIES}")
endif()

add_definitions(
  -DNS3_WOSS_SUPPORT
  -DWOSS_MULTITHREAD
)

include_directories(${WOSS_INCLUDE_DIRS})

# At this point we should have either the directory to woss sources, the
# woss_lib or the cdf_lib So we can start working on the woss-ns3 module

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
    ${WOSS_LIBRARIES}
    ${NETCDF_LIBRARIES}
  TEST_SOURCES test/woss-test.cc
)
