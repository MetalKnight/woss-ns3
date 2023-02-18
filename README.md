# woss-ns3
WOSS ns3 Integration Framework

This repository aims to introduce WOSS support for ns3 users.

WOSS is a multi-threaded C++ framework that permits the integration of any existing underwater channel simulator that expects environmental data as input and provides as output a channel realization.
Currently, WOSS integrates the Bellhop ray-tracing program.
Thanks to its automation the user only has to specify the location in the world and the time where the simulation should take place. This is done by setting the simulated date and the wanted latitude and longitude of every node involved. The simulator automatically handles the rest (see technical description).
WOSS can be integrated in any network simulator or simulation tool that supports C++.

'woss-ns3' relies on the following libraries:
- WOSS
- Acoustic Toolbox
- NetCDF 4 with HDF5 or NetCDF legacy (optional)

Latest WOSS source code, installation instructions and related libraries can be found at http://telecom.dei.unipd.it/ns/woss/

How to install 'woss-ns3' with *waf*:
- download and install the recommended Acoustic Toolbox library
- with optional NetCDF support, download and install the recommended HDF5, NetCDF libraries, with NetCDF4 format support
- download and install the latest WOSS library with optional NetCDF4 and HDF5 support
- clone this repository in the `<ns3-dir>/src` path and then run one of the following:
  - NetCDF4 and HDF5 support, *pay attention to the CXXFLAGS inline redefinition due to a unresolved NetCDF-C++4 API warning* `./waf configure
--with-woss-source=<woss_source_path> --with-woss-library=<woss_lib_path> --with-netcdf4-install=<netcdf4_and_hdf5_installed_path> CXXFLAGS="-Wall -Werror -Wno-unused-variable"`
  - with no NetCDF support `./waf configure --with-woss-source=<woss_source_path> --with-woss-library=<woss_lib_path>`
- for info on how to install all the required libraries with the suggested paths, please check http://telecom.dei.unipd.it/ns/woss/doxygen/installation.html

How to install 'woss-ns3' with *cmake*:
- run one of the following:
  - NetCDF4 and HDF5 support, *pay attention to the CXXFLAGS inline redefinition due to a unresolved NetCDF-C++4 API warning* `CXXFLAGS="-Wno-unused-variable" ./ns3 configure --enable-examples --enable-tests -- -DNS3_WITH_WOSS_SOURCE=/<woss_source_path> -DNS3_WITH_WOSS_LIBRARY=<woss_lib_path> -DNS3_WITH_NETCDF4_INSTALL=<netcdf4_and_hdf5_installed_path>`
  - with no NetCDF support `CXXFLAGS="-Wno-unused-variable" ./ns3 configure --enable-examples --enable-tests -- -DNS3_WITH_WOSS_SOURCE=/<woss_source_path> -DNS3_WITH_WOSS_LIBRARY=<woss_lib_path>`
- for info on how to install all the required libraries with the suggested paths, please check http://telecom.dei.unipd.it/ns/woss/doxygen/installation.html

*Any issue should be reported via github Issue tracker or via e-mail to WOSS@guerra-tlc.com*.
