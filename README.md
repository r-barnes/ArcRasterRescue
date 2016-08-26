ArcRasterRescue
===============

The enclosed files extract raster data from an ArcGIS File Geodatabase into a
GeoTIFF file.

To use the software, run `make` to build `arc_raster.exe`.

List the rasters in the geodatabase using

    ./arc_raster.exe path/to/geodatabase.gdb/

Extract to a GeoTIFF using

    ./arc_raster.exe path/to/geodatabase.gdb/ <RASTER NUM>

Requirements
============

*C++11
*The [RichDEM](https://github.com/r-barnes/richdem) library ("dev" branch)

TODO
====

*Adapt for data types other than float.
*Improve endian checking.
*Check for other kinds of compressions.
*User-friendliate it all.

Credits
=======

* Even Roualt did much of the work figuring out the FileGeodatabase specification. His notes are [here](https://github.com/rouault/dump_gdbtable/wiki/FGDB-Spec). I converted his [dump_gdbtable](https://github.com/rouault/dump_gdbtable) to C++ as a base for this work and then reorganized the code extensively while extending it to read the multiple database tables that represent the raster.

* James Ramm did some exploratory work that resulted in determining that the raster data (at least in cases he tested) had been compressed using zlib. ([Link](http://lists.osgeo.org/pipermail/gdal-dev/2016-July/044761.html))