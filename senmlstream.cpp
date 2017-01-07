#include "senmlstream.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define KM(a,b) const string SenMLStream::a = string(b)


KM(SML_BASENAME,"bn");
KM(SML_BASETIME,"bt");
KM(SML_BASEUNIT,"bu");
KM(SML_BASEVALUE,"bv");
KM(SML_BASESUM,"bs");
KM(SML_VERSION,"bver");
KM(SML_NAME,"n");
KM(SML_UNIT,"u");
KM(SML_VALUE,"v");
KM(SML_STR_VALUE,"vs");
KM(SML_BOOL_VALUE,"vb");
KM(SML_DATA_VALUE,"vd");
KM(SML_POSITION,"pos");
KM(SML_UPDATE_TIME,"ut");
KM(SML_TIME,"t");
KM(SML_LINK,"l");
KM(SML_SUM_VALUE,"s");

#define AKM(a,b) const string SenMLStreamAgSense::a = string(b)

AKM(SML_VI,"vi");
AKM(SML_VI_CAM,"cam");
AKM(SML_VI_EXP,"exp");
AKM(SML_VI_RES,"res");
AKM(SML_VI_IRC,"irc");


bool SenMLStream::stream_reader(cmp_ctx_t * ctx, void * data, size_t limit){

    if (((StreamWrapper*) ctx->buf)->available(limit)) {
        return ((StreamWrapper*) ctx->buf)->read((uint8_t*)data,limit);
    }
    return false;
}

size_t SenMLStream::stream_writer(cmp_ctx_t * ctx, const void * data, size_t count){
        return ((StreamWrapper*) ctx->buf)->write((uint8_t*)data,count);
}


SenMLStream::SenMLStream(StreamWrapper * stream) : _stream(stream)  {
    static_assert( (int)(END * 10) < (TABLE_SIZE-10),"Hash Table too small! Need to increase key ");
    numRecords = 0;
    cmp_init(&cmp, (void*) _stream, SenMLStream::stream_reader, SenMLStream::stream_writer);
}

void SenMLStream::begin(int baud){
      _sending = false;
      _stream->begin(baud);
}

bool SenMLStream::parseField(string key,int r) {
    string sval;
    float fval = nanf("");
    int res =0;

    if ( IS_KEY(key,SML_NAME) ||
         IS_KEY(key,SML_UNIT) ||
         IS_KEY(key,SML_LINK) ||
         IS_KEY(key,SML_STR_VALUE) ) {
         res =readString(sval,SML_VAL_SIZE);
         if (res == -1)
            return false;
         if (!res)
            put(key,string("null"),r);
         else
            put(key,sval,r);
    }

    if ( IS_KEY(key,SML_VALUE) ||
         IS_KEY(key,SML_SUM_VALUE) ||
         IS_KEY(key,SML_UPDATE_TIME) ||
         IS_KEY(key,SML_BOOL_VALUE) ||
         IS_KEY(key,SML_TIME) ) {
         if (res == -1)
            return false;
         if (!res)
            put(key,string("\0"),r);
         else
            put(key,fval,r);
    }
    
    return true;  
};


bool SenMLStream::parseHeader() {
    uint32_t msize=0;
    string k,v;

    
    uint32_t maps = readMap();
    for(uint32_t j=0; j < maps;  j++){
        msize++;
        if (!readString(k,SML_KEY_SIZE))
            return false;
        
        if (k == SML_BASENAME) {
            if (!readString(v,SML_VAL_SIZE))
                return false;
            put(SML_BASENAME,v);
        }
    }

    return msize >0;
}


bool SenMLStream::loop() {
    uint32_t asize;
    
    if (available()) // has existing map
        return true;
    
    if (!cmp_read_array(&cmp, &asize)){
        return false;
    }

    // parse record
    for(uint32_t i=0;i<asize;i++){
        if (!i){
            if (!this->parseHeader())
                return false;
            #ifdef SMLDEBUG
            printf("parsed header\n");
            #endif
        } else {      
            if (!this->parseRecord(i))
                return false;
            #ifdef SMLDEBUG
            printf("parsed record %d\n",i);
            #endif
            numRecords++;
        }
        #ifdef SMLDEBUG
        fflush(stdout); 
        #endif
    }
    return available();
}

