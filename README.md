# woss
WOSS NS3 Integration Framework

This repository aims to introduce WOSS support for NS3 users.

WOSS is a multi-threaded C++ framework that permits the integration of any existing underwater channel simulator that expects environmental data as input and provides as output a channel realization. 
Currently, WOSS integrates the Bellhop ray-tracing program. 
Thanks to its automation the user only has to specify the location in the world and the time where the simulation should take place. This is done by setting the simulated date and the wanted latitude and longitude of every node involved. The simulator automatically handles the rest (see technical description). 
WOSS can be integrated in any network simulator or simulation tool that supports C++.

latest WOSS source code, installation instructions and related libraries can be found at http://telecom.dei.unipd.it/woss
