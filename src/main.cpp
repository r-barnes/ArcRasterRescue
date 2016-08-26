#include "gdal_priv.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <ctime>
#include <byteswap.h>
#include <zlib.h>
#include <type_traits>
#include <cstring>
#include "richdem/common/Array2D.hpp"

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

class Raster {
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
  std::string name;
  std::string alias;
  int8_t      type;
  bool        nullable;
  Raster      raster;
  Shape       shape;
  void print() const {
    std::cout<<"Name     = "<<name      <<"\n"
             <<"Alias    = "<<alias     <<"\n"
             <<"Type     = "<<(int)type <<"\n"
             <<"Nullable = "<<nullable  <<"\n";
  }
};

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



void ReadFields(std::ifstream &gdbtablx, std::ifstream &fin, int nfields){



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

template<class T>
void WriteBlock(std::vector<T> &block, std::vector<T> &raster, int tx, int ty, int block_width, int block_height){

}




void ExtractRaster(std::string filename, int rrd_factor){

}





int main(int argc, char **argv){
  if(argc!=2){
    std::cerr<<"Syntax: "<<argv[0]<<" <File Geodatabase>"<<std::endl;
    return -1;
  }

  Array2D<float> geodata(10000,10000);

  std::string filename  = argv[1];
  std::string filenamex = filename.substr(0,filename.size()-1)+"x";
  std::ifstream gdbtablx(filenamex, std::ios_base::in | std::ios_base::binary);

  if(!gdbtablx.good()){
    std::cerr<<"Could not find '"<<filenamex<<"'!"<<std::endl;
    return -1;
  }

  gdbtablx.seekg(8);
  auto nfeaturesx         = ReadInt32(gdbtablx);
  auto size_tablx_offsets = ReadInt32(gdbtablx);

  std::ifstream gdbtable(filename, std::ios_base::in | std::ios_base::binary);

  if(!gdbtable.good()){
    std::cerr<<"Could not find '"<<filename<<"'!"<<std::endl;
    return -1;
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

  std::cout<<"nfeaturesx         = "<<nfeaturesx           <<"\n";
  std::cout<<"size_tablx_offsets = "<<size_tablx_offsets   <<"\n";
  std::cout<<"nfeatures          = "<<nfeatures            <<"\n";
  std::cout<<"header_offset      = "<<header_offset        <<"\n";
  std::cout<<"header_length      = "<<header_length        <<"\n";
  std::cout<<"layer_geom_type    = "<<(int)layer_geom_type <<"\n";
  std::cout<<"nfields            = "<<nfields              <<"\n";
















  std::vector<Field> fields;
  bool has_flags      = false;
  int nullable_fields = 0;

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

    std::cout<<"\n\nField Number = "<<(fields.size()-1)<<"\n";
    field.print();
  }


  std::vector<float> output(10000*10000);


  for(int f=0;f<nfeaturesx;f++){
    GotoPosition(gdbtablx, 16 + f * size_tablx_offsets);
    auto feature_offset = ReadInt32(gdbtablx);

    if(feature_offset==0)
      continue;

    GotoPosition(gdbtable, feature_offset);

    auto blob_len = ReadInt32(gdbtable);

    std::vector<uint8_t> flags;
    if(has_flags){
      auto nremainingflags = nullable_fields;
      while(nremainingflags>0){
        auto tempflag = ReadByte(gdbtable);
        flags.push_back(tempflag);
        nremainingflags -= 8;
      }
    }


    int row_nbr    = -1;
    int col_nbr    = -1;
    int rrd_factor = -1;



    for(unsigned int fi=0;fi<fields.size();fi++){
      uint8_t ifield_for_flag_test = 0;

      if(has_flags && fields[fi].nullable){
        uint8_t test = (flags[ifield_for_flag_test >> 3] & (1 << (ifield_for_flag_test % 8)));
        ifield_for_flag_test++;
        if(test!=0){
          //print('Field %s : NULL' % fields[ifield].name)
          continue;
        }
      }

      if(fields[fi].type==0){
        auto val = ReadInt16(gdbtable);


      } else if(fields[fi].type==1){
        auto val = ReadInt32(gdbtable);
        if(fields[fi].name=="col_nbr")
          col_nbr = val;
        else if(fields[fi].name=="row_nbr")
          row_nbr = val;
        else if(fields[fi].name=="rrd_factor")
          rrd_factor = val;


      } else if(fields[fi].type==2){
        auto val = ReadFloat32(gdbtable);


      } else if(fields[fi].type==3){
        auto val = ReadFloat64(gdbtable);


      } else if(fields[fi].type == 4 || fields[fi].type == 12){
        auto length = ReadVarUint(gdbtable);
        auto val    = ReadBytes(gdbtable, length);


      } else if(fields[fi].type == 5){
        auto val = ReadFloat64(gdbtable);
        //print('Field %s : %f days since 1899/12/30' % (fields[fi].name, val))
      }

      else if(fields[fi].type == 8){ //Appears to be where raster data is stored
        auto length = ReadVarUint(gdbtable);
        //Skip that which is not a base layer
        if(rrd_factor!=0){
          AdvanceBytes(gdbtable, length);
          continue;
        }

        auto val = ReadBytes(gdbtable, length);

        //Decompress data (TODO: Look for other compression formats)
        std::vector<uint8_t> decompressed(120000);

        Zinflate(val, decompressed);

        //Drop trailer
        //TODO: Don't assume 128x128 block size with 4 byte data
        decompressed.resize(sizeof(float)*128*128);

        //std::cout<<"\n\n";

        //TODO: Generalize for various data formats
        //TODO: Are double and short int also big endian?

        // std::cout<<"Deompcressed: ";
        // for(unsigned int i=0;i<decompressed.size();i++)
        //   std::cout<<(int)decompressed[i]<<" ";
        // std::cout<<"\n";

        auto unpacked = Unpack4BigEndian<float>(decompressed, 128, 128);

        // std::cout<<"Unpacked: ";
        // for(unsigned int i=0;i<unpacked.size();i++)
        //   std::cout<<unpacked[i]<<" ";
        // std::cout<<"\n";

        //Save data to the numpy array
        for(int y=0;y<128;y++)
        for(int x=0;x<128;x++)
          geodata( col_nbr*128+x, row_nbr*128+y ) = unpacked[y*128+x];


      } else if(fields[fi].type == 9){
        auto length = ReadInt32(gdbtable);


      } else if(fields[fi].type == 10 || fields[fi].type == 11){
        auto val = ReadBytes(gdbtable, 16);
        //print('Field %s : "%s"' % (fields[ifield].name, ''.join(x.encode('hex') for x in val)))


      } else if(fields[fi].type == 7){
        std::cerr<<"Unimplemented!"<<std::endl;
/*
          auto geom_len = ReadVarUint(gdbtable);
          auto saved_offset = gdbtable.tellg();

          auto geom_type = ReadVarUint(gdbtable);

          if(geom_type&0xff==50){
            auto nb_total_points = ReadVarUint(gdbtable);
            if(nb_total_points==0){
              GotoPosition(gdbtable, saved_offset + geom_len);
              continue;
            }

            auto nb_geoms = ReadVarUint(gdbtable);

            //TODO ? Conditionnally or unconditionnally present ?
            if( (geom_type & 0x20000000) != 0 ){
                nb_curves = read_varuint(f)
                print("nb_curves: %d" % nb_curves)

              read_bbox(f)
              tab_nb_points = read_tab_nbpoints(f, nb_geoms, nb_total_points)
              read_tab_xy(f, nb_geoms, tab_nb_points)

              if (geom_type & 0x80000000) != 0:
                  read_tab_z(f, nb_geoms, tab_nb_points)

              if (geom_type & 0x40000000) != 0:
                  read_tab_m(f, nb_geoms, tab_nb_points)

              if (geom_type & 0x20000000) != 0:
                  read_curves(f)
            }

          if geom_type & 0xff == 51:

              nb_total_points = read_varuint(f)
              print("nb_total_points: %d" % nb_total_points)
              if nb_total_points == 0:
                  f.seek(saved_offset + geom_len, 0)
                  continue

              nb_geoms = read_varuint(f)
              print("nb_geoms: %d" % nb_geoms)

              # TODO ? Conditionnally or unconditionnally present ?
              if (geom_type & 0x20000000) != 0:
                  nb_curves = read_varuint(f)
                  print("nb_curves: %d" % nb_curves)

              read_bbox(f)
              tab_nb_points = read_tab_nbpoints(f, nb_geoms, nb_total_points)
              read_tab_xy(f, nb_geoms, tab_nb_points)

              if (geom_type & 0x80000000) != 0:
                  read_tab_z(f, nb_geoms, tab_nb_points)

              if (geom_type & 0x40000000) != 0:
                  read_tab_m(f, nb_geoms, tab_nb_points)

              if (geom_type & 0x20000000) != 0:
                  read_curves(f)

              #print("actual_length = %d vs %d" % (f.tell() - saved_offset, geom_len))

          if geom_type & 0xff == 54:

              nb_total_points = read_varuint(f)
              print("nb_total_points: %d" % nb_total_points)
              if nb_total_points == 0:
                  f.seek(saved_offset + geom_len, 0)
                  continue

              # what's that ???
              magic = read_varuint(f)
              print('magic = %d' % magic)

              nb_geoms = read_varuint(f)
              print("nb_geoms: %d" % nb_geoms)

              read_bbox(f)
              tab_nb_points = read_tab_nbpoints(f, nb_geoms, nb_total_points)

              subgeomtype = []
              for i_part in range(nb_geoms):
                  type = read_varuint(f)
                  # only keep lower 4 bits. See extended-shapefile-format.pdf
                  # page 8. Above bits are for priority, material index
                  type = type & 0xf
                  print("type[%d] = %d (%s)" % (i_part, type, multipatch_part_type_to_str(type)))
                  subgeomtype.append(type)

              read_tab_xy(f, nb_geoms, tab_nb_points)

              if (geom_type & 0x80000000) != 0:
                  read_tab_z(f, nb_geoms, tab_nb_points)

              if (geom_type & 0x40000000) != 0:
                  read_tab_m(f, nb_geoms, tab_nb_points)

              #print("actual_length = %d vs %d" % (f.tell() - saved_offset, geom_len))

          if geom_type == 8 or geom_type == 18 or geom_type == 20:
              nb_total_points = read_varuint(f)
              print("nb_total_points: %d" % nb_total_points)
              if nb_total_points == 0:
                  f.seek(saved_offset + geom_len, 0)
                  continue

              read_bbox(f)

              dx_int = dy_int = 0
              for i in range(nb_total_points):
                  vi = read_varint(f)
                  dx_int = dx_int + vi
                  x = dx_int / xyscale + xorig
                  vi = read_varint(f)
                  dy_int = dy_int + vi
                  y = dy_int / xyscale + yorig
                  print("[%d] x=%.15f y=%.15f" % (i, x, y))

              if geom_type == 18 or geom_type == 20:
                  dz_int = 0
                  for i in range(nb_total_points):
                      vi = read_varint(f)
                      dz_int = dz_int + vi
                      z = dz_int / zscale + zorig
                      print("[%d] z=%.15f" % (i, z))

          if geom_type == 1:
              vi = read_varuint(f) - 1
              x0 = vi / xyscale + xorig
              vi = read_varuint(f) - 1
              y0 = vi / xyscale + yorig
              print("%.15f %.15f" % (x0, y0))

          if geom_type == 9:
              vi = read_varuint(f) - 1
              x0 = vi / xyscale + xorig
              vi = read_varuint(f) - 1
              y0 = vi / xyscale + yorig
              vi = read_varuint(f) - 1
              z0 = vi / zscale + zorig
              print("%.15f %.15f %.15f" % (x0, y0, z0))

          if geom_type == 3 or geom_type == 5 or geom_type == 10 or geom_type == 13 or geom_type == 23 or geom_type == 19:

              nb_total_points = read_varuint(f)
              print("nb_total_points: %d" % nb_total_points)
              if nb_total_points == 0:
                  f.seek(saved_offset + geom_len, 0)
                  continue
              nb_geoms = read_varuint(f)
              print("nb_geoms: %d" % nb_geoms)

              read_bbox(f)
              tab_nb_points = read_tab_nbpoints(f, nb_geoms, nb_total_points)
              read_tab_xy(f, nb_geoms, tab_nb_points)

              # z
              if geom_type == 10 or geom_type == 13 or geom_type == 19:
                  read_tab_z(f, nb_geoms, tab_nb_points)

              print('cur_offset = %d' % f.tell())

          f.seek(saved_offset + geom_len, 0)
*/
      } else {
        std::cerr<<"Unhandled type!"<<std::endl;
      }

    }
  }

  geodata.saveGDAL("/z/out.tif");
}




