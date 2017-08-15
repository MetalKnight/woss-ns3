WOSS Integration Framework
--------------------------

WOSS is a multi-threaded C++ framework that permits the integration of any existing underwater channel simulator 
that expects environmental data as input and provides as output a channel realization. 
Currently, WOSS integrates the Bellhop [1]_ ray-tracing program. 
Thanks to its automation the user only has to specify the location in the world and the time 
where the simulation should take place. 
This is done by setting the simulated date and the wanted latitude and longitude of every node involved. 
The simulator automatically handles the rest (see [2]_ and its technical description [3]_).
WOSS has been already integrated in NS-Miracle [4]_ a popular |ns2| extension developed at University of Padua, Italy.

Model Description
*****************

The source code for the Woss Integration Framework lives in the directory
``src/WOSS/``

The WOSS Integration Framework is composed of two main parts:

* the wrapper classes, located in ``src/WOSS/definitions`` containing all the wrappers needed by
  the WOSS framework in order to be fully integrated into the host network simulator. These classes enable 
  the WOSS framework to use the host's model of time, random generator, mobility, spatial coordinates etc...

* the UAN related classes, used to embed the WOSS framework within the UAN framework 
  (channel, propagation, Power Delay Profile representation etc..)


License
*******

WOSS and WOSS Integration Framework are licensed under the GPLv2 license.



Design
======

WOSS Propagation Model
######################
the ``ns3::WossPropModel`` interface extends the API defined in ``ns3::UanPropModel``.
The propagation model provide a power delay profile (PDP) and pathloss
information.  The PDP is retrieved using the ``GetPdpVector`` method which returns type ``ns3::UanPdpVector`` 
which is a c++ vector of ``ns3::UanPdp``. This new function exploits the multithreaded capability
of the WOSS framework, thus allowing the concurrent simulation of all the acoustic channels between 
the given transmitter node and receiver nodes.
the ``GetDelay`` function computes the acoustic propagation delay between two geographical coordinates 
``woss::CoordZ``. The delay represents the first channel tap that exceeds the input SNR threshold. 

WOSS position allocators
########################

the WOSS Integration Framework extends the standard |ns3| position allocators in order to work with geographical
coordinates ``woss::CoordZ``:

#. ``ns3::WossListPositionAllocator`` - list allocator

#. ``ns3::WossGridPositionAllocator`` - grid allocator

#. ``ns3::WossRandomRectanglePositionAllocator`` - random 2D rectangle allocator

#. ``ns3::WossRandomBoxPositionAllocator`` - random 3D box allocator

#. ``ns3::WossRandomDiscPositionAllocator`` - random disc allocator

#. ``ns3::WossUniformDiscPositionAllocator`` - uniform disc allocator

WOSS mobility models
####################

The ``ns3::WossWaypointMobilityModel`` extends the ``ns3::WaypointMobilityModel`` allowing the user to
to use geographical coordinates

How to Install
==============
#. install Bellhop [1]_ and put the binary path in the ``$PATH`` environment;
#. install the NETCDF [6]_ library if you want to use the WOSS environmental databases;
#. install WOSS, with multithread support (mandatory). |ns2| and Ns-Miracle [4]_ support are optional. NETCDF support must be installed depending on previous step.
#. for more info on the previous step please see [7]_ 
#. compile NS3 with WOSS support enabled. Run: ``./waf -d debug --enable-tests --enable-examples --with-woss-source=<woss_source_path> --with-woss-library=<woss_lib_path> --with-netcdf-lib=<netcdf_installed_lib_path> --with-netcdf-include=<netcdf_installed_include_path> configure``

where:
#. ``--with-woss-source=<woss_source_path>`` is mandatory
#. ``--with-woss-library=<woss_lib_path>`` is mandatory
#. ``--with-netcdf-lib=<netcdf_installed_lib_path>`` is optional
#. ``--with-netcdf-include=<netcdf_installed_include_path>`` is optional

Future Work
===========

Extend the framework in order to use the WOSS acoustic transducer simulation in order to compute realistic
power consumption in both transimtting and receiving phases.

References
==========

.. [1] Bellhop source code, URL: http://oalib.hlsresearch.com/Modes/AcousticsToolbox
.. [2] P. Casari, C. Tapparello, F. Guerra, F. Favaro, I. Calabrese, G. Toso, S. Azad, R. Masiero, M. Zorzi; LOTT, R.; Open-source Suites for Underwater Networking: WOSS and DESERT Underwater, URL: http://telecom.dei.unipd.it/media/download/413
.. [3] WOSS Technical Description, URL: http://telecom.dei.unipd.it/ns/woss/doxygen
.. [4] NS-Miracle source code, URL: http://telecom.dei.unipd.it/pages/read/58
.. [5] University of Padua list of publications on Underwater acoustic Networks; URL: http://telecom.dei.unipd.it/pages/read/75/
.. [6] NETCDF library, URL: http://www.unidata.ucar.edu/downloads/netcdf/index.jsp
.. [7] WOSS installation how-to, URL: URL: http://telecom.dei.unipd.it/ns/woss/doxygen/installation.html

Usage
*****

The main way that users who write simulation scripts will typically
interact with the WOSS Framework is through the helper API and through
the publicly visible attributes of the model.

The helper API is defined in ``src/woss/helper/woss-helper.{cc,h}``

The example folder ``src/woss/examples/`` contain some basic code that shows how to set up and use the models.
further examples can be found into the Unit tests in ``src/woss/test/...cc``

Examples
========

An example of the Framework's usage can be found into the examples folder. 

WOSS integration Example
########################

* ``woss-aloha-example``:
    We show how to use the WOSS helper, along with the introduced position allocators 
    and waypoint mobility. We also show how to create a custom acoustic environment, by creating 
    a bathymetrical grid and a custom sediment layer.


Helpers
=======

In this section we give an overview of the available helpers and their behaviour.


WossHelper
##########

This helper:
* automatically configures and initializes all the ``woss::*`` related classes (debug, working directory, Bellhop configuration, simulation parameters etc...)
* configures the environmental databases, either by using custom defined within the simulation or by using the world wide databases provided by WOSS. 

Attributes
==========

.. note::

    TODO

Tracing
=======

.. note::

    TODO

Logging
=======

.. note::

    TODO

Caveats
=======

.. note::

    TODO

Validation
**********

The WOSS framework and its Bellhop integration has been validated by theoritical comparisons and 
sea trials. For more information see [5]_

