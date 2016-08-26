#ifndef _arr_hpp_
#define _arr_hpp_

#include <string>
#include <fstream>
#include <vector>
#include "richdem/common/Array2D.hpp"

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
  Array2D<T> geodata;

  RasterData(std::string filename, const Raster &r);
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