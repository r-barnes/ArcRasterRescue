cmake_minimum_required(VERSION 3.5)

project(arc_raster_rescue LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_HASH)

string(TIMESTAMP MY_TIMESTAMP "%Y-%m-%d %H:%M:%S")

find_package(GDAL REQUIRED)
find_package(ZLIB REQUIRED)

option(EXPLORE "Whether or not to print exploratory debugging info useful for building new features.")

add_library(arc_raster_rescue
  src/arr.cpp
)
target_include_directories(arc_raster_rescue PUBLIC ${GDAL_INCLUDE_DIR} include)
target_link_libraries(arc_raster_rescue PRIVATE ZLIB::ZLIB ${GDAL_LIBRARY})
target_compile_options(arc_raster_rescue PRIVATE
  -Wall -pedantic -Wno-unused-variable
)
target_compile_features(arc_raster_rescue PRIVATE cxx_constexpr)
target_compile_definitions(arc_raster_rescue PUBLIC
  GIT_HASH=\"${GIT_HASH}\"
  COMPILE_TIME=\"${MY_TIMESTAMP}\"
)
if(EXPLORE)
  target_compile_definitions(arc_raster_rescue PRIVATE EXPLORE)
endif()



add_executable(arc_raster_rescue.exe
  evaluation/main.cpp
)
target_link_libraries(arc_raster_rescue.exe PRIVATE arc_raster_rescue)
target_compile_options(arc_raster_rescue.exe PRIVATE
  -Wall -pedantic -Wno-unused-variable
)
target_compile_features(arc_raster_rescue.exe PRIVATE cxx_constexpr)
