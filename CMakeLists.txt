set(NS3_WITH_WOSS_SOURCE ""
    CACHE PATH
          "Path to WOSS source code, for WOSS Integration Framework support"
)
set(NS3_WITH_WOSS_LIBRARY ""
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

set(NS3_WOSS "OFF" CACHE INTERNAL "ON if WOSS can be built")

if(NS3_WITH_WOSS_SOURCE)
  if(NOT (EXISTS ${NS3_WITH_WOSS_SOURCE}))
    message(STATUS "WOSS source was not found in ${NS3_WITH_WOSS_SOURCE}")
  else()
    message(STATUS "WOSS source was found")
    set(NS3_WOSS "ON" CACHE INTERNAL "ON if woss-ns3 can be built")
  endif()
endif()

if(NS3_WITH_WOSS_LIBRARY)

  find_library(
    woss_lib WOSS PATHS ${NS3_WITH_WOSS_LIBRARY}
    PATH_SUFFIXES /build /build/lib /lib /woss /woss/woss_def
  )
  set(woss_headers location-definitions.h woss-manager.h woss-db-manager.h)
  set(woss_header_directories)
  foreach(header ${woss_headers})
    find_file(woss_header_${header} ${header} HINTS ${NS3_WITH_WOSS_LIBRARY}
              PATH_SUFFIXES /build /build/lib /lib /woss /woss/woss_def
                            /woss/woss_db
    )
    if(NOT woss_header_${header})
      message(FATAL_ERROR "Missing WOSS header ${header}")
    endif()
    get_filename_component(
      woss_include_folder ${woss_header_${header}} DIRECTORY
    )
    list(APPEND woss_header_directories ${woss_include_folder})
  endforeach()
  include_directories(${woss_header_directories})

  if(NOT woss_lib)
    message(STATUS "WOSS library was not found in ${NS3_WITH_WOSS_LIBRARY}")
  else()
    message(STATUS "WOSS library was found in ${woss_lib}")
    set(NS3_WOSS "ON" CACHE INTERNAL "ON if woss-ns3 can be built")
  endif()
endif()

# If we can't find WOSS sources or library, don't process this module
if(NOT ${NS3_WOSS})
  message(STATUS "Skipping contrib/woss-ns3")
  return()
endif()

if(NS3_WITH_NETCDF_INSTALL)
  find_library(
    netcdf_lib netcdf PATHS ${NS3_WITH_NETCDF_INSTALL}
    PATH_SUFFIXES /build /build/lib /lib
  )
  if(NOT netcdf_lib)
    message(FATAL_ERROR "NETCDF was not found in ${NS3_WITH_NETCDF_INSTALL}")
  else()
    message(STATUS "NETCDF was found")
  endif()
endif()

if(NS3_WITH_NETCDF4_INSTALL)
  find_library(
    netcdf_lib netcdf PATHS ${NS3_WITH_NETCDF4_INSTALL}
    PATH_SUFFIXES / /build /build/lib /lib
  )
  if(NOT netcdf_lib)
    message(FATAL_ERROR "NETCDF was not found in ${NS3_WITH_NETCDF4_INSTALL}")
  else()
    message(STATUS "NETCDF was found")
  endif()
endif()

add_definitions(-DNS3_WOSS_SUPPORT -DWOSS_MULTITHREAD)

# At this point we should have either the directory to woss sources, the
# woss_lib or the cdf_lib So we can start working on the woss-ns3 module
set(name woss-ns3)

set(source_files
    model/definitions/woss-location.cc
    model/definitions/woss-time-reference.cc
    model/definitions/woss-random-generator.cc
    model/woss-prop-model.cc
    model/woss-channel.cc
    model/woss-position-allocator.cc
    model/woss-waypoint-mobility-model.cc
    helper/woss-helper.cc
)

set(header_files
    model/definitions/woss-location.h
    model/definitions/woss-time-reference.h
    model/definitions/woss-random-generator.h
    model/woss-prop-model.h
    model/woss-channel.h
    model/woss-position-allocator.h
    model/woss-waypoint-mobility-model.h
    helper/woss-helper.h
)

# link to dependencies
set(libraries_to_link
    ${libnetanim}
    ${libnetwork}
    ${libenergy}
    ${libmobility}
    ${libuan}
    ${woss_lib}
    ${netcdf_lib}
)

set(test_sources test/woss-test.cc)

build_contrib_lib(
  "${name}" "${source_files}" "${header_files}" "${libraries_to_link}"
  "${test_sources}"
)
