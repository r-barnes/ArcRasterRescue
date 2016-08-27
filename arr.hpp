#ifndef _arr_hpp_
#define _arr_hpp_

#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#include "gdal_priv.h"
#include <typeinfo>
#include <chrono>


class RasterFields {
 public:
  double raster_mtolerance;
  double raster_xytolerance;
  double raster_zorig;
  double raster_morig;
  double raster_mscale;
  double raster_zscale;
  double raster_xorig;
  double raster_yorig;
  double raster_xyscale;
  double raster_ztolerance;
  bool   raster_has_m;
  bool   raster_has_z;
  std::string wkt;
  std::string raster_column;
};

class Shape {
 public:
  double ymax;
  double xmax;
  double xmin;
  double ymin;
  double morig;
  double zorig;
  double zscale;
  double mscale;
  double xyscale;
  double xorig;
  double yorig;
  bool has_z;
  bool has_m;
  double mtolerance;
  double ztolerance;
  double xytolerance;
  std::string wkt;
};

class Field {
 public:
  std::string  name;
  std::string  alias;
  int8_t       type;
  bool         nullable;
  RasterFields raster;
  Shape        shape;
  void print() const;
};


class BaseTable {
 public:
  std::ifstream gdbtable, gdbtablx;
  int32_t nfeaturesx;
  int32_t size_tablx_offsets;
  std::vector<Field> fields;
  bool has_flags;
  int nullable_fields;

  std::vector<uint8_t> flags;

  std::string getFilenameX(std::string filename);

  void getFlags();

  bool skipField(const Field &field, uint8_t &ifield_for_flag_test);

  BaseTable(std::string filename);
};



class MasterTable : public BaseTable {
 public:
  std::vector< std::pair<std::string, int> > rasters;

  MasterTable(std::string filename);
};

class RasterBase : public BaseTable {
 public:
  int32_t block_width;
  int32_t block_height;

  RasterBase(std::string filename);
};

class RasterProjection : public BaseTable {
 public:
  RasterProjection(std::string filename);
};

class Raster;

template<class T>
class RasterData : public BaseTable {
 public:
  std::vector<T> geodata;

  RasterData(std::string filename, const Raster &r);

  T   no_data;
  std::vector<double> geotransform;
  std::string projection;
  int width;
  int height;
  void resize(int width, int height);
  T& operator()(int x, int y);
  T  operator()(int x, int y) const;
  void setAll(T val);

  GDALDataType myGDALType() const {
    if(typeid(T)==typeid(uint8_t))
      return GDT_Byte;
    else if(typeid(T)==typeid(uint16_t))
      return GDT_UInt16;
    else if(typeid(T)==typeid(int16_t))
      return GDT_Int16;
    else if(typeid(T)==typeid(uint32_t))
      return GDT_UInt32;
    else if(typeid(T)==typeid(int32_t))
      return GDT_Int32;
    else if(typeid(T)==typeid(float))
      return GDT_Float32;
    else if(typeid(T)==typeid(double))
      return GDT_Float64;
    else {
      std::cerr<<"Could not map native type '"<<typeid(T).name()<<"' to GDAL type! (Use `c++filt -t` to decode.)"<<std::endl;
      throw std::runtime_error("Could not map native data type to GDAL type!");
    }
    return GDT_Unknown;
  }

  void save(std::string filename, std::string metadata, bool compress) {
    char **papszOptions = NULL;
    if(compress){
      papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "DEFLATE" );
      papszOptions = CSLSetNameValue( papszOptions, "ZLEVEL",   "6" );
    }

    GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if(poDriver==NULL){
      std::cerr<<"Could not open GDAL driver!"<<std::endl;
      throw std::runtime_error("Could not open GDAL driver!");
    }
    GDALDataset *fout    = poDriver->Create(filename.c_str(), width, height, 1, myGDALType(), papszOptions);
    if(fout==NULL){
      std::cerr<<"Could not open file '"<<filename<<"' for GDAL save!"<<std::endl;
      throw std::runtime_error("Could not open file for GDAL save!");
    }

    GDALRasterBand *oband = fout->GetRasterBand(1);
    oband->SetNoDataValue(no_data);

    //This could be used to copy metadata
    //poDstDS->SetMetadata( poSrcDS->GetMetadata() );

    //TIFFTAG_SOFTWARE
    //TIFFTAG_ARTIST
    {
      std::time_t the_time = std::time(nullptr);
      char time_str[64];
      std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S UTC", std::gmtime(&the_time));
      fout->SetMetadataItem("TIFFTAG_DATETIME",   time_str);
      fout->SetMetadataItem("TIFFTAG_SOFTWARE",   "Arc Raster Rescue (Richard Barnes - rbarnes@umn.edu)");

      auto out_processing_history = std::string(time_str) + " | " + "Arc Raster Rescue (Richard Barnes - rbarnes@umn.edu)" + " | ";
      if(!metadata.empty())
        out_processing_history += metadata;
      else
        out_processing_history += "Unspecified Operation";

      fout->SetMetadataItem("PROCESSING_HISTORY", out_processing_history.c_str());
    }

    //The geotransform maps each grid cell to a point in an affine-transformed
    //projection of the actual terrain. The geostransform is specified as follows:
    //    Xgeo = GT(0) + Xpixel*GT(1) + Yline*GT(2)
    //    Ygeo = GT(3) + Xpixel*GT(4) + Yline*GT(5)
    //In case of north up images, the GT(2) and GT(4) coefficients are zero, and
    //the GT(1) is pixel width, and GT(5) is pixel height. The (GT(0),GT(3))
    //position is the top left corner of the top left pixel of the raster.

    if(!geotransform.empty()){
      if(geotransform.size()!=6){
        std::cerr<<"Geotransform of output is not the right size. Found "<<geotransform.size()<<" expected 6."<<std::endl;
        throw std::runtime_error("saveGDAL(): Invalid output geotransform.");
      }

      fout->SetGeoTransform(geotransform.data());
    }

    if(!projection.empty())
      fout->SetProjection(projection.c_str());

    #ifdef DEBUG
      std::cerr<<"Filename: "<<std::setw(20)<<filename<<" Geotrans0: "<<std::setw(10)<<std::setprecision(10)<<std::fixed<<geotransform[0]<<" Geotrans3: "<<std::setw(10)<<std::setprecision(10)<<std::fixed<<geotransform[3]<< std::endl;
    #endif

    auto temp = oband->RasterIO(GF_Write, 0, 0, width, height, geodata.data(), width, height, myGDALType(), 0, 0);
    if(temp!=CE_None)
      std::cerr<<"Error writing file! Continuing in the hopes that some work can be salvaged."<<std::endl;

    GDALClose(fout);
  }
};


class Raster {
 private:
  std::string hexify(int raster_num);

 public:
  RasterBase        *rb;
  RasterProjection  *rp;
  RasterData<float> *rd;
  Raster(std::string basename, int raster_num);

  ~Raster();
};



#endif