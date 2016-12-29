
#ifndef SENMLSTREAM_H
#define SENMLSTREAM_H

#ifdef _SIMULATOR
#include <string>
using namespace std;
#endif

#include "buffered-serial.h"
#include "sys/types.h"
#include "cmp.h"

#define boolean bool
#define MAX_SENML_RECS 4

#define SML_NAME "n"
#define SML_VALUE "v"
#define SML_UNIT "u"
#define SML_BASENAME "bn"
#define SML_MIN "min"
#define SML_MAX "max"

class SenMLHeader {
    protected:
        string _bn;
        float _pos[3],_v,_min,_max;
        uint32_t bt;

    public:
        SenMLHeader(){};
        SenMLHeader(string bn) { this->_bn = bn;};
        void setBN(string bn) { this->_bn = bn;};
        string getBN() { return this->_bn; };
        void reset(){_bn=""; };
};

class SenMLRecord {
    private:
        string _b,_n,_u;
        uint16_t _sz; // size of image
        uint8_t *_vd;
        float _min,_max,_v;
    
    protected:
        SenMLHeader *_meta;

    public:
        SenMLRecord() : _meta() {};
        SenMLRecord(SenMLHeader * meta) { _meta = meta; };
        
        void reset() {
          _meta = NULL;
          _b = "";
          _n = "";
          _v = 0;
          _vd = NULL;
          _sz = 0;
        };

        void clone(SenMLRecord * smin) {
            memcpy(this->_meta,smin->_meta,sizeof(_meta));
            this->_b  = smin->_b;
            this->_n  = smin->_n;
            this->_v  = smin->_v;
            this->_vd = smin->_vd;
            this->_sz = smin->_sz;
        };
        
        void setMeta(SenMLHeader * meta){_meta = meta;};
        SenMLHeader * getHeader() { return _meta; };
        string getBN() { return (!_meta) ? "" : _meta->getBN(); };
        void setName(string n){_n = n;};
        void setUnit(string u){_u = u;};
        void setValue(float v){_v = v;};
        void setMin(float min){_min = min;};
        void setMax(float max){_max = max;};
        string getName() {return _n;};
        string getUnit() {return _u;};
        float getValue() {return _v;};
        float getMin() {return _min;};
        float getMax() {return _max;};
};


class SenMLStream {

private:    
    SenMLHeader sH;
    SenMLRecord senml[MAX_SENML_RECS];
    uint8_t numRecords;
    boolean _sending;
    
    cmp_ctx_t cmp;
    static bool stream_reader(cmp_ctx_t * ctx, void * data, size_t limit);
    static size_t stream_writer(cmp_ctx_s *ctx, const void *data,
                                                size_t count);
    BufferedEscapedLinuxSerialWrapper *_stream;
    boolean parseHeader();
    boolean parseRecord();

public:
    void setMaxBufAllowed(unsigned int max);
    SenMLStream(BufferedEscapedLinuxSerialWrapper *stream);//{ _stream.begin(9600)}
    bool begin(int baud);
    void reset() { numRecords = 0;};
    bool available() { return numRecords > 0; };
    uint8_t length( ) {return numRecords; };
    SenMLRecord * record(uint8_t i) {
        return numRecords > 0 && i < numRecords ? &senml[i] : NULL;
    };
    
    SenMLRecord * createRecord() {
        if (numRecords >= MAX_SENML_RECS)
            return NULL;
        senml[numRecords].setMeta(&sH);
        return &senml[numRecords++];
    };
    
    bool addRecord(SenMLRecord * sR) {
        if (numRecords >= MAX_SENML_RECS)
            return false;
        senml[numRecords++].clone(sR);
        return true;
    };

    void flush();
    boolean loop();
    
    boolean writeSenML(SenMLRecord *sR,uint8_t numRecs = 1){
        SenMLHeader *sH = sR->getHeader();
        if (!sR)
            return false;

        _stream->beginPacket();

        if (!cmp_write_array(&cmp, numRecs+1))
            return false;

        if (sH->getBN() != ""){
            if (!appendMaps(1)) // Change to check for valid values first
                return false;
            if (!appendMap(string(SML_BASENAME),sH->getBN()))
                return false;
        }   
        _sending = true;
        return true; // Fix
    };
    
    bool appendMaps(uint8_t nmaps){
        return cmp_write_map(&cmp, nmaps);
    };

    bool appendMap(const string key,string val){
        return cmp_write_str(&cmp, key.c_str(),key.length()) && 
               cmp_write_str(&cmp, val.c_str(),val.length());
    };

    bool appendMap(const string key,float val){
        if (!cmp_write_str(&cmp, key.c_str(),key.length()))
            return false;

        if (!cmp_write_float(&cmp, val))
            return false;
        return true;
    };

    bool appendMap(const string key, const uint8_t * p, uint32_t s){
        if (!cmp_write_str(&cmp, key.c_str(),key.length()))
            return false;

        if (!cmp_write_bin_marker(&cmp, s))
            return false;
        return true;
    };

    bool appendBinary(const uint8_t * p, size_t s,boolean end = false){
        if (!cmp.write(&cmp, p,s))
            return false;
        if (end)
            this->flush();
    
        return true;
    };

    
    
    void print(){
      printf("\n\nPrinting:\n");
      for(int i=0;i< numRecords;i++){
        SenMLRecord *r = record(i);
        if (!r)
            break;
        printf("bn => %s\n",r->getBN().c_str());
        printf("n => %s\n",r->getName().c_str());
        printf("u => %s\n",r->getUnit().c_str());
      }
    };
};

#endif
