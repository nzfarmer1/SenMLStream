
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
//      #define bool boolean
#define stof(a) a.toFloat()
#include <WSTring.h>
#define string String
#include <HashMap.h>
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


#define IS_KEY(k,v) (string(k) == v)

class SenMLStream {

    public:
        static const string SML_BASENAME;
        static const string SML_BASETIME;
        static const string SML_BASEUNIT;
        static const string SML_BASEVALUE;
        static const string SML_BASESUM;
        static const string SML_VERSION;
        static const string SML_NAME;
        static const string SML_UNIT;
        static const string SML_VALUE;
        static const string SML_STR_VALUE;
        static const string SML_BOOL_VALUE;
        static const string SML_DATA_VALUE;
        static const string SML_POSITION;
        static const string SML_UPDATE_TIME;
        static const string SML_TIME;
        static const string SML_LINK;
        static const string SML_SUM_VALUE;
    
    protected:
        static enum KEYS  {
            SML_BASENAME_IDX,
            SML_BASETIME_IDX,
            SML_BASEUNIT_IDX,
            SML_BASEVALUE_IDX,
            SML_BASESUM_IDX,
            SML_VERSION_IDX,
            SML_NAME_IDX,
            SML_UNIT_IDX,
            SML_VALUE_IDX,
            SML_STR_VALUE_IDX,
            SML_BOOL_VALUE_IDX,
            SML_DATA_VALUE_IDX,
            SML_POSITION_IDX,
            SML_UPDATE_TIME_IDX,
            SML_TIME_IDX,
            SML_LINK_IDX,
            SML_SUM_VALUE_IDX,
            END} ikeys;
    
    private:

        struct NumKey {
            unsigned long operator()(const uint8_t& k) const
            {
                return  (unsigned long) k%TABLE_SIZE;
            }
        };

        uint8_t numMaps;
        uint8_t numRecords;
        bool _sending;
        
        HashMap<uint8_t, string, NumKey> hmap;
        cmp_ctx_t cmp;
        static bool stream_reader(cmp_ctx_t * ctx, void * data, size_t limit);
        static size_t stream_writer(cmp_ctx_s *ctx, const void *data,
                                                    size_t count);
        StreamWrapper *Stream(){return (StreamWrapper *)cmp.buf;};
        
        StreamWrapper *_stream;
        bool parseHeader();

    protected:
        virtual bool parseRecord(int r) = 0;

        virtual string khash(uint8_t k) = 0;
    
        static string _khash(KEYS k){
            switch(k) {
                case SML_BASENAME_IDX:
                    return SML_BASENAME;
                case SML_BASETIME_IDX:
                    return SML_BASETIME;
                case SML_BASEUNIT_IDX:
                    return SML_BASEUNIT;
                case SML_BASEVALUE_IDX:
                    return SML_BASEVALUE;
                case SML_BASESUM_IDX:
                    return SML_BASESUM;
                case SML_VERSION_IDX:
                    return SML_VERSION;
                case SML_NAME_IDX:
                    return SML_NAME;
                case SML_UNIT_IDX:
                    return SML_UNIT;
                case SML_VALUE_IDX:
                    return SML_VALUE;
                case SML_STR_VALUE_IDX:
                    return SML_STR_VALUE;
                case SML_BOOL_VALUE_IDX:
                    return SML_BOOL_VALUE;
                case SML_DATA_VALUE_IDX:
                    return SML_DATA_VALUE;
                case SML_POSITION_IDX:
                    return SML_POSITION;
                case SML_UPDATE_TIME_IDX:
                    return SML_UPDATE_TIME;
                case SML_TIME_IDX:
                    return SML_TIME;
                case SML_LINK_IDX:       
                    return SML_LINK;
                case SML_SUM_VALUE_IDX:
                    return SML_SUM_VALUE;
                default:
                    return string("unk");
            }
        
          return string("unk");  
        };
    
    
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
            #ifdef SMLDEBUG
            printf("Read [%s]\n",_str);
            #endif
            s = string(_str);
            return true;
        };
    
        bool readNumber(float &f) {
            cmp_object_t obj;
            if (!cmp_read_object(&cmp, &obj))
                return false;
            #ifdef SMLDEBUG
            printf("Read Number Got: %x\n",obj.type);
            #endif
                  
            switch(obj.type) {
                case CMP_TYPE_POSITIVE_FIXNUM:
                    f = (float) obj.as.u8; 
                    break;
                case CMP_TYPE_FLOAT:
                    f = obj.as.flt;
                    break;
                case CMP_TYPE_BOOLEAN:
                    f = (float)obj.as.boolean;
                    break;
                case CMP_TYPE_UINT8:
                    f = (float)obj.as.u8;
                    break;
                case CMP_TYPE_NEGATIVE_FIXNUM:
                    f = (float) obj.as.s8;
                    break;

                default:
                    return false;
            }
            
            return true;
        };
    
        bool parseField(string key,int r);

        virtual  uint8_t key(const string k,int r=0) = 0;

        void put(const string k, const string &v,int r=0) {
          string _v = v;
          uint8_t _k = key(k,r);
          hmap.put(_k,_v);
          #ifdef SMLDEBUG
          printf("PUT [%s] =>[%s]\n",k.c_str(),v.c_str());
          #endif
        };
        
        void put(const string k, const float &v,int r=0) {
        #ifdef _SIMULATOR
          put(k,to_string((long double)v),r);
        #else
          put(k,v,r);
        #endif
        
        };
    
        void put(const string k, const int &v,int r=0) {
        #ifdef _SIMULATOR
          put(k,to_string((long long)v),r);  
        #else
          put(k,v,r);
        #endif
        };
    
        void put(const string k, const uint8_t &v,int r=0) {
        #ifdef _SIMULATOR
          put(k,to_string((long long)v),r);  
        #else
          put(k,v,r);
        #endif
        };
    
        void put(const string k, const uint16_t &v,int r=0) {
        #ifdef _SIMULATOR
          put(k,to_string((long long)v),r);  
        #else
          put(k,v,r);
        #endif
        };
    
        void put(const string k, const bool &v,int r=0) {
        #ifdef _SIMULATOR
          put(k,to_string((long long)v),r);  
        #else
          put(k,v,r);
        #endif
        };
        
    public:
        bool get(const string k, string &v,int r=0) {
          uint8_t _k =key(k,r);
          return hmap.get(_k,v);  
        };
    
        bool get(const string k,  float &v,int r=0) {
          uint8_t _key = key(k,r);
          string _val;
          if (hmap.get(_key,_val)){
              v = (float)stof(_val);
              return true;
          }
          return false;
        };
    
        bool get(const string k,  uint8_t &v,int r=0) {
          uint8_t _key = key(k,r);
          string _val;
          if (hmap.get(_key,_val)){
              v = (uint8_t)stof(_val);
              return true;
          }
          return false;
        };
    
        bool get(const string k,  bool &v,int r=0) {
          uint8_t _key = key(k,r);
          string _val;
          if (hmap.get(_key,_val)){
              v = (bool)stof(_val);
              return true;
          }
          return false;
        };
    
        
        SenMLStream(StreamWrapper *stream);
    
        void begin(int baud);
        void reset() { numRecords = 0; numMaps=0; hmap.clear();};
        bool available() { return numRecords > 0; };
        uint8_t length( ) {return numRecords; };
        
        bool loop();
        
        bool writeSenML(uint8_t numRecs = 1) {
            
            _stream->beginPacket();
    
            if (!cmp_write_array(&cmp, numRecs))
                return false;
    
            _sending = true;
            return true; // Fix
        };
        
        bool appendRecord(uint8_t nmaps) {
            return cmp_write_map(&cmp, nmaps);
        };
    
        bool appendMap(const string key,string val) {
            if (!cmp_write_str(&cmp, key.c_str(),key.length()))
                return false;
            if (!cmp_write_str(&cmp,val.c_str(),val.length()))
                return false;
            return true;
        };
    
        bool appendMap(const string key,bool val) {
            if (!cmp_write_str(&cmp, key.c_str(),key.length()))
                return false;
            if (!cmp_write_bool(&cmp,val))
                return false;
            return true;
        };
    
        bool appendMap(const string key,uint8_t val) {
            if (!cmp_write_str(&cmp, key.c_str(),key.length()))
                return false;
    
            if (!cmp_write_uint(&cmp, val))
                return false;
            return true;
        };
    
        bool appendMap(const string key,uint16_t val) {
            if (!cmp_write_str(&cmp, key.c_str(),key.length()))
                return false;
    
            if (!cmp_write_uint(&cmp, val))
                return false;
            return true;
        };
    
        bool appendMap(const string key,float val) {
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
        

        static void iter(const uint8_t &key,const string &val){
          printf("[%d] [%s] => [%s]\n",key,_khash((KEYS)(key/10)).c_str(),val.c_str());
        };

        static void _iter(const string &key,const string &val){
          printf("[%s] => [%s]\n",key.c_str(),val.c_str());
        };
        
        void print() {
          printf("\n\nPrinting %d:\n",numRecords);
          hmap.iter(iter);
        };
};



/* ============================================================== 

    Core
    
 ============================================================== */


class SenMLStreamCore: public SenMLStream {

    protected:
        string khash(uint8_t k){
            return SenMLStream::_khash((SenMLStream::KEYS)k);
        }

        uint8_t key(const string k,int r=0){
            uint8_t _k =0;
                while((KEYS)_k < END ) {
                    if (khash((KEYS)_k) == k)
                        break;
                    _k++;
                }

            return _k*10 + r;  
        };
    
        bool parseRecord(int r) {
            string k;
        
            uint32_t maps = readMap();
            for(uint32_t j=0; j< maps; j++) {
        
                if (!readString(k,SML_KEY_SIZE))
                    return false;
                
                if (!parseField(k,r))
                    return false;
            }
            return true;    
        }

    public:
        SenMLStreamCore(StreamWrapper * stream) : SenMLStream(stream) { reset(); }
        
        bool loop() {
            return  SenMLStream::loop();
        }
    
};



/* ============================================================== 

    AgSense
    
 ============================================================== */



class SenMLStreamAgSense: public SenMLStream {

    public:
        static const string SML_VI; 
        static const string SML_VI_CAM; 
        static const string SML_VI_EXP; // exposure int
        static const string SML_VI_RES;  // resolution hex
        static const string SML_VI_IRC;// IR Cut (boolean)
        
    protected:
        // Override KEYS to support custom types
        static enum KEYS  {
            SML_VI_IDX = SenMLStream::END,
            SML_VI_CAM_IDX,
            SML_VI_EXP_IDX,
            SML_VI_RES_IDX,
            SML_VI_IRC_IDX,
            END} ikeys;

    
        uint8_t key(const string k,int r=0){
            uint8_t _k =0;
                while((KEYS)_k < END ) {
                    if (khash((KEYS)_k) == k)
                        break;
                    _k++;
                }

            return _k*10 + r;  
        };
    
    
        // Override khash to support custom types
        string khash(uint8_t k) {
            KEYS _k = (KEYS)k;
            if (_k < (SenMLStreamAgSense::KEYS)SenMLStream::END)
                return SenMLStream::_khash((SenMLStream::KEYS)_k);
            
            switch(_k) {
                case SML_VI_IDX:
                    return SML_VI;
                case SML_VI_CAM_IDX:
                    return SML_VI_CAM;
                case SML_VI_RES_IDX:
                    return SML_VI_RES;
                case SML_VI_IRC_IDX:
                    return SML_VI_IRC;
                case SML_VI_EXP_IDX:
                    return SML_VI_EXP;
                default:
                    return string("unk");
            }
            return "unk";
        };
    
        // Custom type
        bool parseVI(int r) {
            string key;
            float fval;
            #ifdef SMLDEBUG
            printf("Parse VI\n");
            #endif 
            uint32_t maps = readMap();
            for(uint32_t j=0; j < maps;  j++){
                if (!readString(key,SML_KEY_SIZE))
                    return false;
        
                if ( IS_KEY(key,SML_VI_EXP) ||
                     IS_KEY(key,SML_VI_RES) ||
                     IS_KEY(key,SML_VI_IRC)) {
                   
                    #ifdef SMLDEBUG
                    printf("Reading Key [%s]\n",key.c_str());
                    #endif
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
        };

        bool parseRecord(int r) {
            string k;
            printf("Parse Record");
            uint32_t maps = readMap();
            for(uint32_t j=0; j< maps; j++) {
        
                if (!readString(k,SML_KEY_SIZE))
                    return false;
                printf("GOT -> %s",k.c_str());
                if (k == SML_VI) // Add support for custom types
                    return parseVI(r);
        
                if (!parseField(k,r)) // Else carry on
                    return false;
            }
            return true;    
        };

    public:
        SenMLStreamAgSense(StreamWrapper * stream) : SenMLStream(stream) { reset(); }
        
        bool loop() {
            return  SenMLStream::loop();
        }

        void reset(){
            SenMLStream::reset();
        };
    
};



#endif
