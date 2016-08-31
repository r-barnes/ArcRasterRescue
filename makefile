CXX=g++
LIBS=`gdal-config --libs` -ljpeg
GDAL_CFLAGS=`gdal-config --cflags`
GIT_HASH=`git rev-parse HEAD`
COMPILE_TIME=`date -u +'%Y-%m-%d %H:%M:%S UTC'`
OPTIM_FLAGS=-O3
DEBUG_FLAGS=-DEXPLORE
CXXFLAGS=$(GDAL_CFLAGS) --std=c++11 -g -Wall -Wno-unused-variable -Wno-unknown-pragmas -DGIT_HASH="\"$(GIT_HASH)\"" -DCOMPILE_TIME="\"$(COMPILE_TIME)\"" 

all:
	$(CXX) $(OPTIM_FLAGS) $(CXXFLAGS) -o arc_raster.exe main.cpp arr.cpp $(LIBS) -lz

debug:
	$(CXX) $(DEBUG_FLAGS) $(CXXFLAGS) -o arc_raster.exe main.cpp arr.cpp $(LIBS) -lz

explore:
	$(CXX) $(OPTIM_FLAGS) -DEXPLORE $(CXXFLAGS) -o arc_raster.exe main.cpp arr.cpp $(LIBS) -lz