#include <zlib.h>
#include <type_traits>
#include <cstring>
#include <sstream>
#include <string>
#include <iomanip>
#include <fstream>
#include <vector>
#include "arr.hpp"

template<class T>
T ReadThing(std::ifstream &fin){
  T v;
  fin.read( reinterpret_cast <char*> (&v), sizeof( T ) );
  return v;
}

uint8_t ReadByte(std::ifstream &fin){
  return ReadThing<uint8_t>(fin);
}

std::vector<uint8_t> ReadBytes(std::ifstream &fin, int count){
  std::vector<uint8_t> ret(count);
  fin.read( reinterpret_cast <char*>(ret.data()), sizeof( uint8_t )*count );
  return ret;
}

std::string ReadBytesAsString(std::ifstream &fin, int count){
  auto v = ReadBytes(fin,count);
  return std::string(v.begin(),v.end());
}

int16_t ReadInt16(std::ifstream &fin){
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ReadThing<int16_t>(fin);
  #else
    #pragma message "Big-endian unimplemented"
  #endif
}

int32_t ReadInt32(std::ifstream &fin){
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ReadThing<int32_t>(fin);
  #else
    #pragma message "Big-endian unimplemented"
  #endif
}

float ReadFloat32(std::ifstream &fin){
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ReadThing<float>(fin);
  #else
    #pragma message "Big-endian unimplemented"
  #endif
}

double ReadFloat64(std::ifstream &fin){
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ReadThing<double>(fin);
  #else
    #pragma message "Big-endian unimplemented"
  #endif
}

uint64_t ReadVarUint(std::ifstream &fin){
  uint64_t shift = 0;
  uint64_t ret   = 0;
  while(true){
    uint8_t b = ReadByte(fin);
    ret      |= ((b & 0x7F) << shift);
    if( (b & 0x80)==0)
      break;
    shift += 7;
  }
  return ret;
}

void AdvanceBytes(std::ifstream &fin, int64_t count){
  fin.seekg(count, std::ios_base::cur);
}

void GotoPosition(std::ifstream &fin, int64_t pos){
  fin.seekg(pos);
}



void Field::print() const {
  std::cout<<"Name     = "<<name      <<"\n"
           <<"Alias    = "<<alias     <<"\n"
           <<"Type     = "<<(int)type <<"\n"
           <<"Nullable = "<<nullable  <<"\n";
}



std::string GetString(std::ifstream &fin, int nbcar=-1){
  std::string temp;

  if(nbcar==-1)
    nbcar = ReadByte(fin);

  for(int j=0;j<nbcar;j++){
    temp += ReadByte(fin);
    AdvanceBytes(fin,1);
  }
  return temp;
}

int32_t GetCount(std::ifstream &fin){
  int32_t count = ReadByte(fin);
  count += ((int)ReadByte(fin))*256;
  return count;
}



void Zinflate(std::vector<uint8_t> &src, std::vector<uint8_t> &dst) {
  z_stream strm  = {0};
  strm.total_in  = strm.avail_in  = src.size();
  strm.total_out = strm.avail_out = dst.size();
  strm.next_in   = (Bytef *) src.data();
  strm.next_out  = (Bytef *) dst.data();

  strm.zalloc = Z_NULL;
  strm.zfree  = Z_NULL;
  strm.opaque = Z_NULL;

  int err = -1;
  int ret = -1;

  err = inflateInit2(&strm, (15 + 32)); //15 window bits, and the +32 tells zlib to to detect if using gzip or zlib
  if(err!=Z_OK){
    inflateEnd(&strm);
    throw std::runtime_error(std::to_string(err));
  }

  err = inflate(&strm, Z_FINISH);
  if (err!=Z_STREAM_END) {
   inflateEnd(&strm);
   throw std::runtime_error(std::to_string(err));
  }

  ret = strm.total_out;
  inflateEnd(&strm);
  dst.resize(ret);
}

template<class T>
std::vector<T> Unpack4BigEndian(std::vector<uint8_t> &packed, const int block_width, const int block_height){
  std::vector<T> output(block_width*block_height);
  packed.resize(sizeof(T)*block_width*block_height);

  if(std::is_same<T,float>::value){
    #if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
      for(unsigned int i=0;i<packed.size();i+=4){
        std::swap(packed[i],  packed[i+3]);
        std::swap(packed[i+1],packed[i+2]);
      }
    #endif
  } else {
    std::cerr<<"Unimplemented type conversion!"<<std::endl;
  }

  memcpy(output.data(), packed.data(), sizeof(T)*block_width*block_height);

  return output;
}




std::string BaseTable::getFilenameX(std::string filename){
  return filename.substr(0,filename.size()-1)+"x";
}

void BaseTable::getFlags(){
  if(has_flags){
    auto nremainingflags = nullable_fields;
    while(nremainingflags>0){
      auto tempflag = ReadByte(gdbtable);
      flags.push_back(tempflag);
      nremainingflags -= 8;
    }
  }
}

bool BaseTable::skipField(const Field &field, uint8_t &ifield_for_flag_test){
  if(has_flags && field.nullable){
    uint8_t test = (flags[ifield_for_flag_test >> 3] & (1 << (ifield_for_flag_test % 8)));
    ifield_for_flag_test++;
    return test!=0;
  }
  return false;
}

BaseTable::BaseTable(std::string filename){
  std::string filenamex = getFilenameX(filename);
  gdbtablx.open(filenamex, std::ios_base::in | std::ios_base::binary);

  if(!gdbtablx.good()){
    std::cerr<<"Could not find '"<<filenamex<<"'!"<<std::endl;
    throw std::runtime_error("Could not find '"+filenamex+"'!");
  }

  gdbtablx.seekg(8);
  nfeaturesx         = ReadInt32(gdbtablx);
  size_tablx_offsets = ReadInt32(gdbtablx);

  gdbtable.open(filename, std::ios_base::in | std::ios_base::binary);

  if(!gdbtable.good()){
    std::cerr<<"Could not find '"<<filename<<"'!"<<std::endl;
    throw std::runtime_error("Could not find '"+filename+"'!");
  }

  gdbtable.seekg(4);
  auto nfeatures = ReadInt32(gdbtable);

  gdbtable.seekg(32);
  auto header_offset = ReadInt32(gdbtable);

  gdbtable.seekg(header_offset);
  auto header_length = ReadInt32(gdbtable);

  AdvanceBytes(gdbtable,4);

  //1 = point
  //2 = multipoint
  //3 = polyline
  //4 = polygon
  //9 = multipatch
  auto layer_geom_type = ReadByte(gdbtable);

  AdvanceBytes(gdbtable,3);

  auto nfields = GetCount(gdbtable);

  // std::cout<<"nfeaturesx         = "<<nfeaturesx           <<"\n";
  // std::cout<<"size_tablx_offsets = "<<size_tablx_offsets   <<"\n";
  // std::cout<<"nfeatures          = "<<nfeatures            <<"\n";
  // std::cout<<"header_offset      = "<<header_offset        <<"\n";
  // std::cout<<"header_length      = "<<header_length        <<"\n";
  // std::cout<<"layer_geom_type    = "<<(int)layer_geom_type <<"\n";
  // std::cout<<"nfields            = "<<nfields              <<"\n";

  has_flags       = false;
  nullable_fields = 0;

  for(int i=0;i<nfields;i++){
    int8_t nbcar;

    auto field = Field();

    field.name     = GetString(gdbtable);
    field.alias    = GetString(gdbtable);
    field.type     = ReadByte(gdbtable);
    field.nullable = true;
    //print('type = %d (%s)' % (type, field_type_to_str(type))) //TODO

    if(field.type==6){        //ObjectID
      auto magic_byte1 = ReadByte(gdbtable);
      auto magic_byte2 = ReadByte(gdbtable);
      field.nullable   = false;
    } else if(field.type==7){ //Shape
      auto magic_byte1 = ReadByte(gdbtable); //0
      auto flag =  ReadByte(gdbtable);       //6 or 7
      if( (flag & 1)==0 )
        field.nullable = false;


      auto wkt_len = GetCount(gdbtable);
      field.shape.wkt = GetString(gdbtable, wkt_len/2);

      auto magic_byte3 = ReadByte(gdbtable);

      field.shape.has_m = false;
      field.shape.has_z = false;
      if(magic_byte3==5)
        field.shape.has_z = true;
      if(magic_byte3==7){
        field.shape.has_m = true;
        field.shape.has_z = true;
      }

      field.shape.xorig   = ReadFloat64(gdbtable);
      field.shape.yorig   = ReadFloat64(gdbtable);
      field.shape.xyscale = ReadFloat64(gdbtable);
      if(field.shape.has_m){
        field.shape.morig = ReadFloat64(gdbtable);
        field.shape.mscale = ReadFloat64(gdbtable);
      }
      if(field.shape.has_z){
        field.shape.zorig = ReadFloat64(gdbtable);
        field.shape.zscale = ReadFloat64(gdbtable);
      }
      field.shape.xytolerance = ReadFloat64(gdbtable);
      if(field.shape.has_m)
        field.shape.mtolerance = ReadFloat64(gdbtable);
      if(field.shape.has_z)
        field.shape.ztolerance = ReadFloat64(gdbtable);

      field.shape.xmin = ReadFloat64(gdbtable);
      field.shape.ymin = ReadFloat64(gdbtable);
      field.shape.xmax = ReadFloat64(gdbtable);
      field.shape.ymax = ReadFloat64(gdbtable);

      while(true){
        auto read5 = ReadBytes(gdbtable,5);
        if(read5[0]!=0 || (read5[1]!=1 && read5[1]!=2 && read5[1]!=3) || read5[2]!=0 || read5[3]!=0 || read5[4]!=0){
          AdvanceBytes(gdbtable,-5);
          auto datum = ReadFloat64(gdbtable);
        } else {
          for(int i=0;i<read5[1];i++)
            auto datum = ReadFloat64(gdbtable);
          break;
        }
      }

    } else if(field.type==4){ //String
      auto width = ReadInt32(gdbtable);
      auto flag = ReadByte(gdbtable);
      if( (flag&1)==0 )
        field.nullable = false;
      auto default_value_length = ReadVarUint(gdbtable);
      if( (flag&4)!=0 && default_value_length>0){
        //auto default_value = ;
        AdvanceBytes(gdbtable, default_value_length);
      }

    } else if(field.type==8){
      AdvanceBytes(gdbtable,1);
      auto flag = ReadByte(gdbtable);
      if( (flag&1)==0 )
        field.nullable = false;

    } else if(field.type==9) { //Raster
      AdvanceBytes(gdbtable,1);
      auto flag = ReadByte(gdbtable);
      if( (flag & 1)==0 )
        field.nullable = false;

      field.raster.raster_column = GetString(gdbtable);

      auto wkt_len     = GetCount(gdbtable);
      field.raster.wkt = GetString(gdbtable,wkt_len/2);

      //f.read(82) //TODO: Was like this in source.

      auto magic_byte3 = ReadByte(gdbtable);

      if(magic_byte3>0){
        field.raster.raster_has_m = false;
        field.raster.raster_has_z = false;
        if(magic_byte3==5)
          field.raster.raster_has_z = true;
        if(magic_byte3==7){
          field.raster.raster_has_m = true;
          field.raster.raster_has_z = true;
        }

        field.raster.raster_xorig   = ReadFloat64(gdbtable);
        field.raster.raster_yorig   = ReadFloat64(gdbtable);
        field.raster.raster_xyscale = ReadFloat64(gdbtable);

        if(field.raster.raster_has_m){
          field.raster.raster_morig  = ReadFloat64(gdbtable);
          field.raster.raster_mscale = ReadFloat64(gdbtable);
        }

        if(field.raster.raster_has_z){
          field.raster.raster_zorig  = ReadFloat64(gdbtable);
          field.raster.raster_zscale = ReadFloat64(gdbtable);
        }

        field.raster.raster_xytolerance  = ReadFloat64(gdbtable);
        if(field.raster.raster_has_m)
          field.raster.raster_mtolerance = ReadFloat64(gdbtable);
        if(field.raster.raster_has_z)
          field.raster.raster_ztolerance = ReadFloat64(gdbtable);
      }

      AdvanceBytes(gdbtable,1);

    } else if(field.type==11 || field.type==10 || field.type==12){ //UUID or XML
      auto width         = ReadByte(gdbtable);
      auto flag          = ReadByte(gdbtable);
      if( (flag&1)==0 )
        field.nullable = false;
    } else {
      auto width    = ReadByte(gdbtable);
      auto flag     = ReadByte(gdbtable);
      if( (flag&1)==0 )
        field.nullable = false;

      auto default_value_length = ReadByte(gdbtable);

      if( (flag&4)!=0 ){
        if(field.type==0 && default_value_length==2)
          auto default_value = ReadInt16(gdbtable);
        else if(field.type==1 && default_value_length==4)
          auto default_value = ReadInt32(gdbtable);
        else if(field.type==2 && default_value_length==4)
          auto default_value = ReadFloat32(gdbtable);
        else if(field.type==3 && default_value_length==8)
          auto default_value = ReadFloat64(gdbtable);
        else if(field.type==5 && default_value_length==8)
          auto default_value = ReadFloat64(gdbtable);
        else
          AdvanceBytes(gdbtable, default_value_length);
      }
    }

    if(field.nullable){
      has_flags        = true;
      nullable_fields += 1;
    }

    if(field.type!=6)
      fields.push_back(field);

    //std::cout<<"\n\nField Number = "<<(fields.size()-1)<<"\n";
    //field.print();
  }


}


MasterTable::MasterTable(std::string filename) : BaseTable(filename) {
  for(int f=0;f<nfeaturesx;f++){
    GotoPosition(gdbtablx, 16 + f * size_tablx_offsets);
    auto feature_offset = ReadInt32(gdbtablx);

    if(feature_offset==0)
      continue;

    GotoPosition(gdbtable, feature_offset);

    auto blob_len = ReadInt32(gdbtable);

    getFlags();

    uint8_t ifield_for_flag_test = 0;
    for(unsigned int fi=0;fi<fields.size();fi++){
      if(skipField(fields[fi],ifield_for_flag_test))
        continue;

      if(fields[fi].type==1){
        auto val = ReadInt32(gdbtable);
      } else if(fields[fi].type == 10 || fields[fi].type == 11){ //10=DatasetGUID
        auto val = ReadBytes(gdbtable, 16);
      } else if(fields[fi].type == 4 || fields[fi].type == 12){  //String
        auto length = ReadVarUint(gdbtable);
        auto val    = ReadBytesAsString(gdbtable, length);
        auto loc    = val.find("fras_ras_");
        std::cerr<<val<<" ("<<std::hex<<f<<")"<<std::endl;
        if(loc!=std::string::npos)
          rasters.emplace_back(val.substr(9), f-1);
      }
    }
  }
}





/* Plus 0
FID = 1
feature_offset = 498
blob_len = 96
flags = [0, 0, 252]
Type: 1
Field  sequence_nbr : 1
Type: 1
Field     raster_id : 1
Type: 4
Field          name : ""
Type: 1
Field    band_flags : 2048
Type: 1
Field    band_width : 1
Type: 1
Field   band_height : 1
Type: 1
Field    band_types : 4195328
Type: 1
Field   block_width : 128
Type: 1
Field  block_height : 128
Type: 3
Field block_origin_x : -178.500000
Type: 3
Field block_origin_y : 88.500000
Type: 3
Field         eminx : -180.000000
Type: 3
Field         eminy : 87.000000
Type: 3
Field         emaxx : -177.000000
Type: 3
Field         emaxy : 90.000000
Type: 1
Field         cdate : 1471710590
Type: 1
Field         mdate : 1471710590
Type: 1
Field          srid : 0
set([1, 3, 4, 6])
*/
RasterBase::RasterBase(std::string filename) : BaseTable(filename) {
  for(int f=0;f<nfeaturesx;f++){
    GotoPosition(gdbtablx, 16 + f * size_tablx_offsets);
    auto feature_offset = ReadInt32(gdbtablx);

    if(feature_offset==0)
      continue;

    GotoPosition(gdbtable, feature_offset);

    auto blob_len = ReadInt32(gdbtable);

    getFlags();

    uint8_t ifield_for_flag_test = 0;
    for(unsigned int fi=0;fi<fields.size();fi++){
      if(skipField(fields[fi], ifield_for_flag_test))
        continue;

      if(fields[fi].type==1){
        auto val = ReadInt32(gdbtable);
        if(fields[fi].name=="block_width")
          block_width = val;
        else if(fields[fi].name=="block_height")
          block_height = val;

      } else if(fields[fi].type == 4 || fields[fi].type == 12){
        auto length = ReadVarUint(gdbtable);
        auto val    = ReadBytes(gdbtable, length);
      } else if(fields[fi].type==3){
        auto val = ReadFloat64(gdbtable);
      }
    }
  }
}

/* Plus 1
#######################
###FILENAME: ../dump_gdbtable/001.gdb/a0000000e.gdbtable
nfeaturesx = 1
size_tablx_offsets = 5
nfeatures = 1
header_offset = 40
header_length = 2056
layer_geom_type = 4
polygon
nfields = 5

field = 0
nbcar = 3
name = OID
nbcar_alias = 0
alias = 
type = 6 (objectid)
magic1 = 4
magic2 = 2
nullable = 0 

field = 1
nbcar = 6
name = RASTER
nbcar_alias = 0
alias = 
type = 9 (raster)
flag = 5
nbcar = 13
raster_column = Raster Column
wkt = PROJCS["NAD83_UTM_zone_15N",GEOGCS["GCS_North_American_1983",DATUM["D_North_American_1983",SPHEROID["GRS_1980",6378137.0,298.257222101]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433]],PROJECTION["Transverse_Mercator"],PARAMETER["false_easting",500000.0],PARAMETER["false_northing",0.0],PARAMETER["central_meridian",-93.0],PARAMETER["scale_factor",0.9996],PARAMETER["latitude_of_origin",0.0],UNIT["Meter",1.0]]
magic3 = 7
xorigin = -5120900.000000000000000
yorigin = -9998100.000000000000000
xyscale = 450445547.391053795814514
morigin = -100000.000000000000000
mscale = 10000.000000000000000
zorigin = -100000.000000000000000
zscale = 10000.000000000000000
xytolerance = 0.001000000000000
mtolerance = 0.001000000000000
ztolerance = 0.001000000000000
1
nullable = 1 

field = 2
nbcar = 9
name = FOOTPRINT
nbcar_alias = 0
alias = 
type = 7 (geometry)
magic1 = 0
flag = 7
wkt = PROJCS["NAD83_UTM_zone_15N",GEOGCS["GCS_North_American_1983",DATUM["D_North_American_1983",SPHEROID["GRS_1980",6378137.0,298.257222101]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433]],PROJECTION["Transverse_Mercator"],PARAMETER["false_easting",500000.0],PARAMETER["false_northing",0.0],PARAMETER["central_meridian",-93.0],PARAMETER["scale_factor",0.9996],PARAMETER["latitude_of_origin",0.0],UNIT["Meter",1.0]]
magic3 = 7
xorigin = -5120900.000000000000000
yorigin = -9998100.000000000000000
xyscale = 450445547.391053795814514
morigin = -100000.000000000000000
mscale = 10000.000000000000000
zorigin = -100000.000000000000000
zscale = 10000.000000000000000
xytolerance = 0.001000000000000
mtolerance = 0.001000000000000
ztolerance = 0.001000000000000
xmin = 421568.000000000000000
ymin = 4872699.000000001862645
xmax = 428822.000000000000000
ymax = 4877607.000000003725290
cur_pos = 2015
0.0
nullable = 1 

field = 3
nbcar = 16
name = FOOTPRINT_Length
nbcar_alias = 0
alias = 
type = 3 (float64)
width = 8
flag = 3
default_value_length = 0
nullable = 1 

field = 4
nbcar = 14
name = FOOTPRINT_Area
nbcar_alias = 0
alias = 
type = 3 (float64)
width = 8
flag = 3
default_value_length = 0
nullable = 1 

------ROWS (FEATURES)------

FID = 1
feature_offset = 2100
blob_len = 101
flags = [240]
Type: 9
Field        RASTER : "" (len=1)
Type: 7
geom_len = 79
geom_type = 5 --> 5
polygon
nb_total_points: 5
nb_geoms: 1
minx = 421568.000000000000000
miny = 4872699.000000001862645
maxx = 428822.000000000640284
maxy = 4877607.000000003725290
nb_points[0] = 5
[1] 421568.000000000000000 4872699.000000001862645
[2] 421568.000000000000000 4877607.000000003725290
[3] 428822.000000000000000 4877607.000000003725290
[4] 428822.000000000000000 4872699.000000001862645
[5] 421568.000000000000000 4872699.000000001862645

cur_offset = 2189
Type: 3
Field FOOTPRINT_Length : 24324.000000
Type: 3
Field FOOTPRINT_Area : 35602632.000014
set([9, 3, 6, 7])
*/
RasterProjection::RasterProjection(std::string filename) : BaseTable(filename){}


std::string Raster::hexify(int raster_num){
  std::stringstream ss;
  ss << "a" << std::setfill('0') << std::setw(8) << std::hex << raster_num << ".gdbtable";
  return ss.str();
}

Raster::Raster(std::string basename, int raster_num){
  rb = new RasterBase       (basename+hexify(raster_num+0));
  rp = new RasterProjection (basename+hexify(raster_num+1));
  rd = new RasterData<float>(basename+hexify(raster_num+4), *this);
}

Raster::~Raster(){
  delete rb;
  delete rp;
}


template<class T>
RasterData<T>::RasterData(std::string filename, const Raster &r) : BaseTable(filename){
  //TODO: Initialize with NoData value
  geodata.resize(10000,10000);

  std::cerr<<"Opening Raster Data as "<<filename<<std::endl;

  std::cerr<<"Raster data"<<std::endl;

  for(int f=0;f<nfeaturesx;f++){
    GotoPosition(gdbtablx, 16 + f * size_tablx_offsets);
    auto feature_offset = ReadInt32(gdbtablx);

    std::cerr<<"\n\n\nf: "<<f<<std::endl;

    if(feature_offset==0)
      continue;

    GotoPosition(gdbtable, feature_offset);

    auto blob_len = ReadInt32(gdbtable);

    getFlags();

    int row_nbr    = -1;
    int col_nbr    = -1;
    int rrd_factor = -1;

    std::cerr<<"Fields.size = "<<fields.size()<<std::endl;

    uint8_t ifield_for_flag_test = 0;
    for(unsigned int fi=0;fi<fields.size();fi++){
      if(skipField(fields[fi], ifield_for_flag_test))
        continue;

      if(fields[fi].type==1){
        auto val = ReadInt32(gdbtable);
        std::cout<<fields[fi].name<<" = "<<val<<std::endl;
        if(fields[fi].name=="col_nbr")
          col_nbr = val;
        else if(fields[fi].name=="row_nbr")
          row_nbr = val;
        else if(fields[fi].name=="rrd_factor")
          rrd_factor = val;
      } else if(fields[fi].type == 4 || fields[fi].type == 12){
        auto length = ReadVarUint(gdbtable);
        auto val    = ReadBytes(gdbtable, length);
        std::cout<<fields[fi].name<<" = ";
        for(auto &v: val)
          std::cout<<v<<" ";
        std::cout<<std::endl;
      } else if(fields[fi].type==8){ //Appears to be where raster data is stored
        std::cerr<<"HERE!"<<std::endl;

        auto length = ReadVarUint(gdbtable);

        std::cerr<<"Length = "<<length<<std::endl;

        //Skip that which is not a base layer
        if(rrd_factor!=0){
          AdvanceBytes(gdbtable, length);
          continue;
        }

        auto val = ReadBytes(gdbtable, length);

        if(val[0]!=120 || val[1]!=156){ //Magic Bytes indicating zlib
          std::cerr<<"Unrecognised data compression format. Only zlib is available!"<<std::endl;
          std::cerr<<"Initial bytes: ";
          for(uint8_t i=0;i<10;i++)
            std::cerr<<(int)val[i]<<" ";
          std::cerr<<std::endl;
          throw std::runtime_error("Unrecognised data compression format. Only zlib is available!");
        }

        //Decompress data (TODO: Look for other compression formats)
        std::vector<uint8_t> decompressed(120000);

        Zinflate(val, decompressed);

        std::cerr<<"decompressed_size = "<<decompressed.size()<<std::endl;

        std::cerr<<sizeof(float)<<" "<<r.rb->block_width<<" "<<r.rb->block_height<<std::endl;

        //Drop trailer
        //TODO: Don't assume 128x128 block size with 4 byte data
        decompressed.resize(sizeof(float)*r.rb->block_width * r.rb->block_height);

        //std::cout<<"\n\n";

        //TODO: Generalize for various data formats
        //TODO: Are double and short int also big endian?

        // std::cout<<"Deompcressed: ";
        // for(unsigned int i=0;i<decompressed.size();i++)
        //   std::cout<<(int)decompressed[i]<<" ";
        // std::cout<<"\n";

        auto unpacked = Unpack4BigEndian<float>(decompressed, r.rb->block_width, r.rb->block_height);

        // std::cout<<"Unpacked: ";
        // for(unsigned int i=0;i<unpacked.size();i++)
        //   std::cout<<unpacked[i]<<" ";
        // std::cout<<"\n";

        //Save data to the numpy array
        for(int y=0;y<r.rb->block_height;y++)
        for(int x=0;x<r.rb->block_width;x++)
          geodata( col_nbr*(r.rb->block_width)+x, row_nbr*(r.rb->block_height)+y ) = unpacked[y*(r.rb->block_width)+x];
      } else {
        std::cerr<<"Unrecognised field type: "<<(int)fields[fi].type<<std::endl;
      }
    }
  }

  geodata.saveGDAL("/z/out.tif");
}
