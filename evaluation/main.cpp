#include <arc_raster_rescue/arc_raster_rescue.hpp>

#include <iomanip>
#include <iostream>
#include <locale>

int main(int argc, char **argv){
  std::string operation;
  for(int i=0;i<argc;i++)
    operation+=argv[i]+std::string(" ");

  std::cerr<<program_identifier<<std::endl;

  if(argc!=2 && argc!=4){
    std::cerr<<"Syntax A: "<<argv[0]<<" <File Geodatabase>"<<std::endl;
    std::cerr<<"Syntax B: "<<argv[0]<<" <File Geodatabase> <Raster> <Output Name>"<<std::endl;
    std::cerr<<"\n";
    std::cerr<<"Syntax A will list all of the rasters in the data set along with selection numbers for use with Syntax B.\n";
    std::cerr<<"Syntax B, given an FGDB and raster selection number, will output the raster to the indicated output file.\n";
    std::cerr<<"Syntax B also accepts raster names, as listed by Syntax A, as inputs for <Raster>\n";
    std::cerr<<"\nNOTE: The geodatabase path must end with a slash!\n";
    std::cerr<<"EXAMPLE: ./arc_raster_rescue.exe path/to/geodatabase.gdb/ dem03 /z/out.tif\n";
    return -1;
  }

  std::string basename = argv[1];

  MasterTable mt(basename+"a00000001.gdbtable");

  if(argc==2){
    std::cout<<"Rasters found: \n";
    for(unsigned int r=0;r<mt.rasters.size();r++){
      std::cout<<std::setw(2)<<r<<" "<<mt.rasters[r].first;
      #ifdef EXPLORE
        std::cout<<" ("<<std::hex<<std::setw(3)<<mt.rasters[r].second<<std::dec<<")"; //Latter part displays file identifier
      #endif
      std::cout<<"\n";
    }
    if(mt.rasters.size()==0){
      std::cout<<"\tNo rasters found!"<<std::endl;
    }
  } else if(argc==4){
    if(mt.rasters.size()==0){
      std::cerr<<"No rasters found!"<<std::endl;
      return -1;
    }

    unsigned int raster_num = (unsigned int)-1;

    try {
      raster_num = std::stoi(argv[2]);
    } catch (...) {
      for(unsigned int i=0;i<mt.rasters.size();i++){
        std::locale loc;
        std::string lower;
        for(auto &s: mt.rasters[i].first)
          s = std::tolower(s,loc);

        if(mt.rasters[i].first==argv[2] || lower==argv[2]){
          raster_num = i;
          break;
        }
      }
    }
    if(raster_num>=mt.rasters.size()){ //Note: Don't need <0 check because raster_num is unsigned
      std::cerr<<"Invalid raster number! Must be 0-"<<(mt.rasters.size()-1)<<"."<<std::endl;
      return -1;
    }

    ExportRasterToGeoTIFF(operation, basename, mt.rasters.at(raster_num).second, std::string(argv[3]));
  }

  return 0;
}