#pragma once
#ifndef SENMLSTREAM_H
#define SENMLSTREAM_H

#ifdef _SIMULATOR // LINUX

#include <string>
using namespace std;
#include "buffered-serial.h"
#include <sys/types.h>
#include <math.h>
#include <assert.h>
#include "../HashMap/src/HashMap.h"
#include "cmp.h"

#define StreamWrapper BufferedEscapedLinuxSerialWrapper

#else // DUINO
#define to_string(a) String(a)
#define stof(a) a.toFloat()
#include <WSTring.h>
#define string String
#include <HashMap.h>
#include <math.h>
#include <xarq.h>
extern "C"{
#include <cmp.h>
};

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
#define SML_VAL_SIZE 255
#define SML_MAX_STR 255

#define IS_KEY(k,v) (string(k) == v)


class SenMLStream {

    private:
        float _lon,_lat, _alt;
        bool  _hasPos() { return !(isnan(_lon) || isnan(_lon) || isnan(_alt)); };

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
        static const string SML_PUBSUB;
    
        SenMLStream(StreamWrapper *stream,string bn = "");
        
        void begin(uint32_t baud);
        void reset() { _numRecords = 0;  hmap.clear();};
        bool available() { return _numRecords > 0; };
        StreamWrapper * stream();
#ifdef _SIMULATOR // LINUX
        bool busy() { return _sending; };
#else
        // Also check to see if underlying Serial stream is busy
        bool busy() { return _sending || _stream->busy();  }; 
#endif        

        uint8_t length() { return _numRecords; };
        bool loop();
        
        void setBN(string bn) {
            this->_bn = bn;
        }

        string getBN() {
            return this->_bn;
        }
        
        void setPos(float lon, float lat, float alt){
          this->_lon = lon;  
          this->_lat = lat;  
          this->_alt = alt;  
        };
        
        /** 
         * convert uint8_t * to string. keep track of length
         **/
        static uint16_t encode(const uint8_t *st,string &s,uint16_t len){
            if (!len)
                return 0;
            const char ESC  = 0x23;
            const char ZERO = 0x24;
            char *sc = (char*) malloc(len*2+3);
            assert(sc !=NULL);
            memcpy(sc+2+len,st,len);
            sc[len] = (uint8_t)len >>8;
            sc[len+1] = (uint8_t)len;
            uint16_t j=0;
            for(uint16_t i=0;i<len+2;i++,j++){
                if (sc[i+len] == ESC){
                    sc[j] = ESC;
                    sc[++j]= ESC;
                    continue;
                }
                if (!sc[i+len]) {
                    sc[j] = ESC;
                    sc[++j] =ZERO;
                    continue;
                }
                sc[j] = sc[i+len]; 
            }
            sc[j]='\0';
	    #ifdef _SIMULATOR
            s = string((char *)sc);
	    #else	
            s = to_string((char *)sc);
	    #endif

            free(sc);
            return j;
        };

        /** 
         * convert encoded uint8_t * as string back to uint8_t.
         **/
        
        static uint16_t decode(const string &s,uint8_t *v) {
            const char ESC  = 0x23;
            const char ZERO = 0x24;
            const char *sc= s.c_str();
            uint16_t j=0;
            uint16_t slen = strlen(sc);
            uint16_t len =0;
            
            if (!slen)
                return 0;
             
            for(uint16_t i=0;i<slen;i++){
                if (sc[i] == ESC) {
                    v[j] = (sc[i+1] == ZERO) ? 0 : sc[i+1];
                    ++i;
                } else {
                    v[j] = sc[i];
                }
                if (j == 2 && len ==0){
                    len  = v[0] << 8;
                    len |= v[1];
                    j=0;
                    i--;
                } else {
                    j++;
                }
            }
            return len;
        }


        int get(const string k, uint8_t *v,uint16_t &len,int r=0) {
            uint8_t _k = key(k,r);
            string s;

            if (!hmap.get(_k,s)){
                len = 0;
                return -1;
            }
            
            len = decode(s,v);
            return  len > 0;
        };

        
        int get(const string k, string &v,int r=0) {
            uint8_t _k =key(k,r);
            if (!hmap.get(_k,v))
              return -1;
            return (v == "null") ? 0 : 1;
        };
    
        int get(const string k,  float &v,int r=0) {
            uint8_t _key = key(k,r);
            string _val;
            if (!hmap.get(_key,_val))
              return -1;
            if (_val == "null")
                return 0;
            v = (float)stof(_val);
            return (isnan(v)) ? 0 : 1;
        };
    
        int get(const string k,  uint8_t &v,int r=0) {
            uint8_t _key = key(k,r);
            string _val;
            if (!hmap.get(_key,_val))
              return -1;
            if (_val == "null")
                return 0;
            v = (uint8_t)stof(_val);
            return (isnan(v)) ? 0 : 1;
        };

        int get(const string k,  uint16_t &v,int r=0) {
            uint8_t _key = key(k,r);
            string _val;
            if (!hmap.get(_key,_val)){
              return -1;
            }
            if (_val == "null")
                return 0;
            v = (uint16_t)stof(_val);
            return (isnan(v)) ? 0 : 1;
        };

        int get(const string k,  uint32_t &v,int r=0) {
            uint8_t _key = key(k,r);
            string _val;
            if (!hmap.get(_key,_val))
              return -1;
            if (_val == "null")
                return 0;
            v = (uint32_t)stof(_val);
            return (isnan(v)) ? 0 : 1;
        };
    
        int get(const string k,  bool &v,int r=0) {
            uint8_t _key = key(k,r);
            string _val;
            if (!hmap.get(_key,_val))
              return -1;
            if (_val == "null")
                return 0;
            v = (bool)stof(_val);
            return (isnan(v)) ? 0 : 1;
        };
    
        
        // Create new out going message
        bool writeSenML(uint8_t numRecs = 1) {
            
            _stream->beginPacket();
            
            if (!cmp_write_array(&cmp, numRecs+1))
                return false;

            uint8_t nm = _hasPos() ? 3 : 2;
            this->appendRecord(nm); // 1 maps

            this->appendMap(this->SML_BASENAME,this->_bn);
            if (_hasPos())
                this->appendPos();
	    #ifdef _SIMULATOR
            this->appendMap(this->SML_BASETIME,to_string((long long unsigned int)time(NULL)));
	    #else
            this->appendMap(this->SML_BASETIME,xmillis());
	    #endif
           
            _sending = true;
            return true; 
        };
        
        bool appendRecord(uint8_t nmaps) {
            return cmp_write_map(&cmp, nmaps);
        };

        bool appendPos() {
            if (!_hasPos())
                return true;
            if (!cmp_write_str(&cmp, "pos",3))
                return false;
            if (!cmp_write_array(&cmp, 3))
                return false;
            if (!cmp_write_float(&cmp, _lon))
                return false;
            if (!cmp_write_float(&cmp, _lat))
                return false;
            if (!cmp_write_float(&cmp, _alt))
                return false;
            return true;
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

        bool appendMap(const string key,uint32_t val) {
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
    
        bool appendMap(const string key, const uint8_t * p, uint32_t sz, bool append=false) {
            if (!cmp_write_str(&cmp, key.c_str(), key.length()))
                return false;
    
            if (!cmp_write_bin_marker(&cmp,(uint32_t)sz))
                return false;
            
            if (append)
                return appendBinary(p,(size_t)sz);
            
            return true;
        };
    
        bool appendBinary(const uint8_t * p, size_t s) {
            if (!cmp.write(&cmp, p, s))
                return false;
        
            return true;
        };
    
        void flush() {
            if (_sending)
                _stream->endPacket();
            _sending = false;
        };
        
        static void iter(const uint8_t &key,const string &val) {
        #if defined(XDEBUG) && (__SMLDEBUG_LINUX)
          printf("[%d] [%s] => [%s]\n",key,_khash((KEYS)(key/10)).c_str(),val.c_str());
        #endif
        };

            
        void print() {
        #if defined(XDEBUG) && (__SMLDEBUG_LINUX)
          printf("\n\nPrinting %d:\n",_numRecords);
         #endif 
          hmap.iter(iter);
        };

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
            SML_PUBSUB_IDX,
            END} ikeys;
    
        static void _iter(const string &key,const string &val){
        #if defined(XDEBUG) && (__SMLDEBUG_LINUX)
            printf("[%s] => [%s]\n",key.c_str(),val.c_str());
        #endif
        };
    
        
        
    private:
        struct NumKey {
            unsigned long operator()(const uint8_t& k) const
            {
                return  (unsigned long) k%TABLE_SIZE;
            }
        };
        HashMap<uint8_t, string, NumKey> hmap;

        uint8_t _numRecords; // Number of records in message (excluding header)
        bool _sending;      // True write message started
        cmp_ctx_t cmp;
        static bool stream_reader(cmp_ctx_t * ctx, void * data, size_t limit);
        static size_t stream_writer(cmp_ctx_s *ctx, const void *data,
                                                    size_t count);

        StreamWrapper *_stream;
        bool parseHeader();

    protected:
        // Override the following to add custom features
        virtual bool parseRecord(int r) = 0; 
        virtual  uint8_t key(const string k,int r=0) = 0; 
        virtual string khash(uint8_t k) = 0;
        bool parseField(string key,int r);
        
        string _bn;
    
        static string _khash(KEYS k) { // Core
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
                case SML_PUBSUB_IDX:
                    return SML_PUBSUB;
                default:
                    return string("unk");
            }
        
            return string("unk");  
        };
    
        // read a Map structure {a:b}    
        uint32_t readMap() {
            uint32_t msize;
            if (!cmp_read_map(&cmp,&msize))
                return 0;
            
            return msize;
        };

        // read a String (handle NULLS)
        int16_t readData(uint8_t *s,uint32_t sz) {
            cmp_object_t obj;

            #if defined(XDEBUG) && (__SMLDEBUG)
            debug.print("Read Object Got: ");
            #endif
            if (!cmp_read_object(&cmp, &obj))
                return -1;
            #if defined(XDEBUG) && (__SMLDEBUG)
                debug.println(obj.type);
            #endif
            
            if (obj.type == CMP_TYPE_NIL || obj.type != CMP_TYPE_BIN8)
                return 0;
            sz = obj.as.bin_size;

            if ( sz > SML_MAX_STR)
                return -1;
            
            if (!cmp.read(&cmp, (void*)s, sz)) {
              return false;
            }
            return sz;
        }
    
        // read a String (handle NULLS)
        int readString(string &s,uint32_t sz) {
            cmp_object_t obj;
            char  _str[SML_MAX_STR];

            #if defined(XDEBUG) && (__SMLDEBUG_LINUX)
            printf("Read Object Got: ");
            #endif
            #if defined(XDEBUG) && (__SMLDEBUG)
            debug.print("Read Object Got: ");
            #endif
            if (!cmp_read_object(&cmp, &obj))
                return -1;
            #if defined(XDEBUG) && (__SMLDEBUG_LINUX)
            printf("%x\n",obj.type);
            #endif
            #if defined(XDEBUG) && (__SMLDEBUG)
            debug.println(obj.type);
            #endif
            
            if (obj.type == CMP_TYPE_NIL)
                return 0;

            if (!cmp_object_to_str(&cmp,&obj,_str,sz))
                return -1;
            
            s = string(_str);
            return 1;
        };
    
        // read a Number (handle NULLS)
        // Note: all numbers parsed into float and stored as string
        int readNumber(float &f) {
            cmp_object_t obj;
            if (!cmp_read_object(&cmp, &obj))
                return false;
            #if defined(XDEBUG) && (__SMLDEBUG_LINUX)
            printf("Read Number Got: %x\n",obj.type);
            #endif
            #if defined(XDEBUG) && (__SMLDEBUG)
            debug.print("Read Number Got: ");
            debug.println(obj.type);
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
                case CMP_TYPE_UINT16:
                    f = (float)obj.as.u16;
                    break;
                case CMP_TYPE_UINT32:
                    f = (float)obj.as.u32;
                    break;
                case CMP_TYPE_UINT64:
                    f = (float)obj.as.u64;
                    break;
                case CMP_TYPE_SINT8:
                    f = (float)obj.as.u8;
                    break;
                case CMP_TYPE_SINT16:
                    f = (float)obj.as.s16;
                    break;
                case CMP_TYPE_SINT32:
                    f = (float)obj.as.s32;
                    break;
                case CMP_TYPE_SINT64:
                    f = (float)obj.as.s64;
                    break;
                case CMP_TYPE_NEGATIVE_FIXNUM:
                    f = (float) obj.as.s8;
                    break;
                case CMP_TYPE_NIL:
                    f = 0;
                    return 0;
                default:
                    return -1;
            }
            return 1;
        };
    
        void put(const string k, const string &v,int r=0) {
            uint8_t _k = key(k,r);
            hmap.put(_k,v);
            #if defined(XDEBUG) && (__SMLDEBUG_LINUX)
            printf("PUT [%s] =>[%s]\n",k.c_str(),v.c_str());
            #elif defined (XDEBUG) && (__SMLDEBUG)
            debug.print("SML PUT ");
            debug.print(k.c_str());
            debug.print(" => ");
            debug.println(v.c_str());
            debug.print("Record: ");
            debug.println(r);
            #endif
        };
        

        void put(const string k, const float &v,int r=0) {
            #ifdef _SIMULATOR
            put(k,to_string((long double)v),r);
            #else
            put(k,string(v),r);
            #endif
        };
    
        void put(const string k, const int &v,int r=0) {
            #ifdef _SIMULATOR
            put(k,to_string((long long)v),r);  
            #else
            put(k,string(v),r);
            #endif
        };

        
        void put(const string k,  const uint8_t *p,uint16_t len,int r=0) {
            string s;
            if (!len)
                put(k,string('\0'),r);  
            encode(p,s,len); // Encode uint8_t as C++ String sans '\0'
            put(k,string(s),r);
        };


        void put(const string k, const uint8_t &v,int r=0) {
            #ifdef _SIMULATOR
            put(k,to_string((long long)v),r);  
            #else
            put(k,string(v),r);
            #endif
        };
    
        void put(const string k, const uint16_t &v,int r=0) {
            #ifdef _SIMULATOR
            put(k,to_string((long long)v),r);  
            #else
            put(k,string(v),r);
            #endif
        };
    
        void put(const string k, const bool &v,int r=0) {
            #ifdef _SIMULATOR
            put(k,to_string((long long)v),r);  
            #else
            put(k,string(v),r);
            #endif
        };

};



/* ============================================================== 

    Core
    
 ============================================================== */


class SenMLStreamCore: public SenMLStream {

    public:
        SenMLStreamCore(StreamWrapper * stream) : SenMLStream(stream) { reset(); }
        
        bool loop() {
            return  SenMLStream::loop();
        }

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
        
                if (readString(k,SML_KEY_SIZE) !=1)
                    return false;
                
                if (!parseField(k,r))
                    return false;
            }
            return true;    
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

        SenMLStreamAgSense(StreamWrapper * stream,string bn = "") : SenMLStream(stream,bn) { reset(); }
        
        bool loop() {
            return  SenMLStream::loop();
        }

        void reset(){
            SenMLStream::reset();
        };

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
            string  key;
            int     res =0;
            float   fval = 0;
            #if defined(XDEBUG) && (__SMLDEBUG_LINUX)
            printf("Parse VI\n");
            #elif defined (XDEBUG) && (__SMLDEBUG)
            debug.println("Parse VI");
            #endif
            
            uint32_t maps = readMap();
            #if defined (XDEBUG) && (__SMLDEBUG)
            debug.print("Got Maps#  ");
            debug.println(maps);
            #endif
            for(uint32_t j=0; j < maps;  j++) {
                if (readString(key,SML_KEY_SIZE) <=0){
                    return false;
                }
        
                #if defined(XDEBUG) && (__SMLDEBUG_LINUX)
                printf("Reading Key [%s]\n",key.c_str());
                #elif defined (XDEBUG) && (__SMLDEBUG)
                debug.print("Reading Key  ");
                debug.println(key.c_str());
                #endif

                if ( IS_KEY(key,SML_VI_EXP) ||
                     IS_KEY(key,SML_VI_RES) ||
                     IS_KEY(key,SML_VI_IRC)) {
                   
                    res = readNumber(fval);
                    #if defined (XDEBUG) && (__SMLDEBUG)
                    debug.print("Result  ");
                    debug.println(res);
                    #endif
                    if (res == -1)
                        return false;
                    
                    if (res == 0) { // NULL
                        put(key,string("null"),r);
                        continue;
                    }

                    #if defined(XDEBUG) && (__SMLDEBUG)
                    debug.print("Got:  ");
                    debug.println((uint8_t)fval);
                    #endif
                    
                    if (IS_KEY(key,SML_VI_EXP))
                        put(key,(uint8_t)fval,r);
                        
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
            uint32_t maps = readMap();
            for(uint32_t j=0; j< maps; j++) {

                if (!readString(k,SML_KEY_SIZE))
                    return false;

                #if defined(XDEBUG) && (__SMLDEBUG_LINUX)
                    printf("GOT -> " );
                    printf("%s\n",k.c_str());
                #endif
                #if defined(XDEBUG) && (__SMLDEBUG)
                    debug.print("GOT -> " );
                    debug.println(k.c_str());
                #endif
                if (k == SML_VI) // Add support for custom types
                    return parseVI(r);
        
                if (!parseField(k,r)) // Else carry on
                    return false;
            }
            return true;    
        };

    
};



#endif // SENMLSTREAM_H
