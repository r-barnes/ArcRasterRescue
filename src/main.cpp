#include <iostream>
#include "arr.hpp"

int main(int argc, char **argv){
  if(argc!=2 && argc!=3){
    std::cerr<<"Syntax: "<<argv[0]<<" <File Geodatabase> [Raster]"<<std::endl;
    return -1;
  }

  std::string basename = argv[1];

  MasterTable mt(basename+"a00000001.gdbtable");

  if(argc==2){
    std::cout<<"Rasters found: \n";
    for(unsigned int r=0;r<mt.rasters.size();r++)
      std::cout<<std::setw(2)<<r<<" "<<mt.rasters[r].first<<" ("<<std::hex<<std::setw(3)<<mt.rasters[r].second<<std::dec<<")\n";
    return 0;
  } else if(argc==3){
    int raster_num = std::stoi(argv[2]);
    Raster raster(basename, mt.rasters[raster_num].second);
  }

  return 0;
}