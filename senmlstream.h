
#ifndef SENMLSTREAM_H
#define SENMLSTREAM_H

#ifdef _SIMULATOR // LINUX

#include <string>
using namespace std;
#include "buffered-serial.h"
#include "sys/types.h"
#include <math.h>
#include "../HashMap/src/HashMap.h"
#include "cmp.h"
#define StreamWrapper BufferedEscapedLinuxSerialWrapper

#else // DUINO
#define bool boolean

#include <WSTring.h>
#define string String
#include <math.h>
#include <xarq.h>
#include <cmp.h>
#define StreamWrapper BufferedEscapedXStreamWrapper

#endif

/*
    +---------------+-------+---------+
    |          Name | label | Type    |
    +---------------+-------+---------+
    |     Base Name | bn    | String  |
    |     Base Time | bt    | Number  |
    |     Base Unit | bu    | String  |
    |    Base Value | bv    | Number  |
    |      Base Sum | bs    | Number  |
    |       Version | bver  | Number  |
    |          Name | n     | String  |
    |          Unit | u     | String  |
    |         Value | v     | Number  |
    |  String Value | vs    | String  |
    | Boolean Value | vb    | Boolean |
    |    Data Value | vd    | String  |
    |     Value Sum | s     | Number  |
    |          Time | t     | Number  |
    |   Update Time | ut    | Number  |
    |          Link | l     | String  |
    +---------------+-------+---------+
*/

#define MAX_SENML_RECS 4
#define SML_KEY_SIZE 4
#define SML_VAL_SIZE 254
#define SML_MAX_STR 254

#define SML_BASENAME "bn"
#define SML_BASETIME "bt"
#define SML_BASEUNIT "bu"
#define SML_BASEVALUE "bv"
#define SML_BASESUM   "bs"
#define SML_VERSION   "bver"
#define SML_NAME "n"
#define SML_UNIT "u"
#define SML_VALUE "v"
#define SML_STR_VALUE "vs"
#define SML_BOOL_VALUE "vb"
#define SML_DATA_VALUE "vd"
#define SML_POSITION "pos"
#define SML_UPD_TIME "ut"
#define SML_TIME "t"
#define SML_LINK "l"
#define SML_SUM_VALUE "s"


#define IS_KEY(k,v) (string(k) == v)


class SenMLStream {

struct StrKey {
    unsigned long operator()(const string& k) const
    {
        int l = 0;
        size_t sz=0; 
        int k1 = stoi(k,&sz);
        string key = k.substr(sz+1,k.length()-(sz+1));
        while(keymaps[l] != "_"){
            if (keymaps[l++] == key)
                break;
        }
        return  (unsigned long) (l*10 + k1)%TABLE_SIZE;
    }
};

private:    
    uint8_t numMaps;
    uint8_t numRecords;
    bool _sending;
    
    HashMap<string, string, StrKey> hmap;
    cmp_ctx_t cmp;
    static bool stream_reader(cmp_ctx_t * ctx, void * data, size_t limit);
    static size_t stream_writer(cmp_ctx_s *ctx, const void *data,
                                                size_t count);
    StreamWrapper *Stream(){return (StreamWrapper *)cmp.buf;};
    
    StreamWrapper *_stream;
    bool parseHeader();
    bool parseRecord(int r);

protected:
    static const string keymaps[];
    
    uint32_t readMap(){
        uint32_t msize;
        if (!cmp_read_map(&cmp,&msize))
            return 0;
        
        return msize;
    };

    bool readString(string &s,uint32_t sz) {
        uint32_t string_size;
        char  _str[SML_MAX_STR];
        if (!cmp_read_str_size(&cmp, &string_size))
                return false;

        if (!string_size){
            s = string("");
            return true;
        }

        if (string_size > sz)
            return false;
        if (!_stream->read((uint8_t*)_str, string_size))
            return false;
        
        _str[string_size] = '\0';
        printf("Read [%s]\n",_str);
        s = string(_str);
        return true;
    };

    bool readNumber(float &f) {
        cmp_object_t obj;
        if (!cmp_read_object(&cmp, &obj))
            return false;
        
        
        switch(obj.type) {
            case CMP_TYPE_POSITIVE_FIXNUM:
                #ifdef SMLDEBUG
                printf("Got FIXNUM: %d\n",obj.as.u8);
                #endif
                f = (float) obj.as.u8; 
                break;
            case CMP_TYPE_FLOAT:
                f = obj.as.flt;
                break;
            case CMP_TYPE_BOOLEAN:
                f = (float)obj.as.boolean;
                break;
            default:
                return false;
        }
        
        return true;
    };

    bool parseField(string key,int r);

    static string __key(const string k,int r=0){
        return to_string((long long)r) + "-" + k;  
    };
    
    void put(const string k, const string &v,int r=0) {
      string _v = v;
      string _k = __key(k,r);
      hmap.put(_k,_v);
      printf("PUT %s\n",_k.c_str());
    };
    
    void put(const string k, const float &v,int r=0) {
      put(k,to_string((long double)v),r);  
    };

    void put(const string k, const int &v,int r=0) {
      put(k,to_string((long long)v),r);  
    };

    void put(const string k, const uint8_t &v,int r=0) {
      put(k,to_string((long long)v),r);  
    };

    void put(const string k, const uint16_t &v,int r=0) {
      put(k,to_string((long long)v),r);  
    };

    void put(const string k, const bool &v,int r=0) {
      put(k,to_string((long long)v),r);  
    };
    
public:
    bool get(const string k, string &v,int r=0) {
      string _k =__key(k,r);
      return hmap.get(_k,v);  
    };

    bool get(const string k,  float &v,int r=0) {
      string _key = __key(k,r);
      string _val;
      if (hmap.get(_key,_val)){
          v = (float)stof(_val);
          return true;
      }
      return false;
    };

    bool get(const string k,  uint8_t &v,int r=0) {
      string _key = __key(k,r);
      string _val;
      if (hmap.get(_key,_val)){
          v = (uint8_t)stof(_val);
          return true;
      }
      return false;
    };

    bool get(const string k,  bool &v,int r=0) {
      string _key = __key(k,r);
      string _val;
      if (hmap.get(_key,_val)){
          v = (bool)stof(_val);
          return true;
      }
      return false;
    };

    
public:
    SenMLStream(StreamWrapper *stream);

    void begin(int baud);
    void reset() { numRecords = 0; numMaps=0; hmap.clear();};
    bool available() { return numRecords > 0; };
    uint8_t length( ) {return numRecords; };
    
    bool loop();
    
    bool writeSenML(uint8_t numRecs = 1){
        
        _stream->beginPacket();

        if (!cmp_write_array(&cmp, numRecs))
            return false;

        _sending = true;
        return true; // Fix
    };
    
    bool appendRecord(uint8_t nmaps){
        return cmp_write_map(&cmp, nmaps);
    };

    bool appendMap(const string key,string val){
        if (!cmp_write_str(&cmp, key.c_str(),key.length()))
            return false;
        if (!cmp_write_str(&cmp,val.c_str(),val.length()))
            return false;
        return true;
    };

        bool appendMap(const string key,bool val){
        if (!cmp_write_str(&cmp, key.c_str(),key.length()))
            return false;
        if (!cmp_write_bool(&cmp,val))
            return false;
        return true;
    };

    bool appendMap(const string key,uint8_t val){
        if (!cmp_write_str(&cmp, key.c_str(),key.length()))
            return false;

        if (!cmp_write_uint(&cmp, val))
            return false;
        return true;
    };

    bool appendMap(const string key,uint16_t val){
        if (!cmp_write_str(&cmp, key.c_str(),key.length()))
            return false;

        if (!cmp_write_uint(&cmp, val))
            return false;
        return true;
    };

    bool appendMap(const string key,float val){
        if (!cmp_write_str(&cmp, key.c_str(),key.length()))
            return false;
        
        if (!isnan(val)){
            if (!cmp_write_float(&cmp, val))
                return false;
        } else {
            if (!cmp_write_str(&cmp,"",0))
                return false;
        }
        return true;
    };

    bool appendMap(const string key, const uint8_t * p, uint32_t s){
        if (!cmp_write_str(&cmp, key.c_str(), key.length()))
            return false;

        if (!cmp_write_bin_marker(&cmp, s))
            return false;
        return true;
    };

    bool appendBinary(const uint8_t * p, size_t s){
        if (!cmp.write(&cmp, p, s))
            return false;
    
        return true;
    };

    void flush() {
        if (_sending)
            _stream->endPacket();
        _sending = false;
    };
    
    static void iter(const string &key,const string &val){
      printf("[%s] => [%s]\n",key.c_str(),val.c_str());
    };
    
    void print() {
      printf("\n\nPrinting %d:\n",numRecords);
      hmap.iter(iter);
    };
};


/* ============================================================== 

    AgSense
    
 ============================================================== */

#define SML_INFO "vi"
#define SML_VI_CAM "cam"
#define SML_VI_EXP "exp" // exposure int
#define SML_VI_RES "res" // resolution hex
#define SML_VI_IRC "irc" // IR Cut (boolean)

class SenMLStreamAgSense: public SenMLStream {

   protected:
    uint16_t exp;
    uint8_t res;
    bool  irc;
    

    bool parseVI(int r) {
        string key;
        float fval;
        printf("Parse VI\n");
    
        uint32_t maps = readMap();
        for(uint32_t j=0; j < maps;  j++){
            if (!readString(key,SML_KEY_SIZE))
                return false;
    
            if ( IS_KEY(key,SML_VI_EXP) ||
                 IS_KEY(key,SML_VI_RES) ||
                 IS_KEY(key,SML_VI_IRC)) {
               
                printf("Reading Key [%s]",key.c_str());
                if (!readNumber(fval))
                    return false;
                if (IS_KEY(key,SML_VI_EXP))
                    put(key,(int)fval,r);
                    
                if (IS_KEY(key,SML_VI_RES))
                    put(key,(uint8_t)fval,r);
    
                if (IS_KEY(key,SML_VI_IRC))
                    put(key,(bool)fval,r);
            }
        }
        
        return true;    
    }

    bool parseRecord(int r) {
        string k;
    
        uint32_t maps = readMap();
        for(uint32_t j=0; j< maps; j++) {
    
            if (!readString(k,SML_KEY_SIZE))
                return false;
    
            if (k == SML_VI_CAM)
                return parseVI(r);
    
            if (!parseField(k,r))
                return false;
        }
        return true;    
    }

    public:
        
        SenMLStreamAgSense(StreamWrapper * stream) : SenMLStream(stream) { reset(); }
        
        void reset(){
            SenMLStream::reset();
            //put(SML_VI_EXP,220);
            //put(SML_VI_RES,0x1C);
            //put(SML_VI_IRC,false);
        };
    
};



#endif
