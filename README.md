ArcRasterRescue
===============

The enclosed program extracts (rescues!) raster data from an ArcGIS File
Geodatabase into a GeoTIFF file.

List the numbers and names of rasters in the geodatabase using

    ./arc_raster.exe path/to/geodatabase.gdb/

Extract to a GeoTIFF using

    ./arc_raster.exe path/to/geodatabase.gdb/ <RASTER NUM> <OUTPUT FILE>

You can also extract a GeoTIFF using

    ./arc_raster.exe path/to/geodatabase.gdb/ <RASTER NAME> <OUTPUT FILE>

The geodatabase path must end with a slash!

Requirements
============

* [C++11](https://en.wikipedia.org/wiki/C%2B%2B11)
* [zlib](http://www.zlib.net/)
* [GDAL](http://www.gdal.org/)

Compilation
===========

To compile run:

    make

Edit the makefile to include the `-DEXPLORE` flag to print additional
information useful for development to stderr as the program runs.

TODO
====

*Improve endian checking.

*Check for other kinds of compressions.

*Improve calculation of raster dimensions.

*Improve geotransform calculations.

Credits
=======

* Even Roualt did much of the work figuring out the FileGeodatabase specification. His notes are [here](https://github.com/rouault/dump_gdbtable/wiki/FGDB-Spec) and a program he wrote for extractin FGDB data is [here](https://github.com/rouault/dump_gdbtable).

* James Ramm did some exploratory work that resulted in determining that the raster data (at least in cases he tested) had been compressed using zlib. ([Link](http://lists.osgeo.org/pipermail/gdal-dev/2016-July/044761.html))

* Richard Barnes converted code by Even Roualt into C++ as a base for Arc Raster Rescue and, starting from some notes by James Ramm, continued deciphering the format. He reorganized the code extensively to increase its modularity, determined how to connect the various raster tables together, drafted geotransform and WKT projection extraction capabilities, determined how data types were specified, and produced a working executable to extract raster data into GeoTIFFs. He also wrote this paragraph.