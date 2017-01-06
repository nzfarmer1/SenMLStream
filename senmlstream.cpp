#include "senmlstream.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


const string SenMLStream::SML_BASENAME = SenMLStream::khash(SML_BASENAME_IDX);
const string SenMLStream::SML_BASETIME = SenMLStream::khash(SML_BASETIME_IDX);
const string SenMLStream::SML_BASEUNIT = SenMLStream::khash(SML_BASEUNIT_IDX);
const string SenMLStream::SML_BASEVALUE = SenMLStream::khash(SML_BASEVALUE_IDX);
const string SenMLStream::SML_BASESUM = SenMLStream::khash(SML_BASESUM_IDX);
const string SenMLStream::SML_VERSION = SenMLStream::khash(SML_VERSION_IDX);
const string SenMLStream::SML_NAME = SenMLStream::khash(SML_NAME_IDX);
const string SenMLStream::SML_UNIT = SenMLStream::khash(SML_UNIT_IDX);
const string SenMLStream::SML_VALUE = SenMLStream::khash(SML_VALUE_IDX);
const string SenMLStream::SML_STR_VALUE = SenMLStream::khash(SML_STR_VALUE_IDX);
const string SenMLStream::SML_BOOL_VALUE = SenMLStream::khash(SML_BOOL_VALUE_IDX);
const string SenMLStream::SML_DATA_VALUE = SenMLStream::khash(SML_DATA_VALUE_IDX);
const string SenMLStream::SML_POSITION = SenMLStream::khash(SML_POSITION_IDX);
const string SenMLStream::SML_UPDATE_TIME = SenMLStream::khash(SML_UPDATE_TIME_IDX);
const string SenMLStream::SML_TIME = SenMLStream::khash(SML_TIME_IDX);
const string SenMLStream::SML_LINK = SenMLStream::khash(SML_LINK_IDX);
const string SenMLStream::SML_SUM_VALUE = SenMLStream::khash(SML_SUM_VALUE_IDX);

const string SenMLStreamAgSense::SML_VI_CAM = SenMLStreamAgSense::khash(SML_VI_CAM_IDX);
const string SenMLStreamAgSense::SML_VI_EXP = SenMLStreamAgSense::khash(SML_VI_EXP_IDX); // "exp"; // exposure int
const string SenMLStreamAgSense::SML_VI_RES = SenMLStreamAgSense::khash(SML_VI_RES_IDX); //"res" // resolution hex
const string SenMLStreamAgSense::SML_VI_IRC = SenMLStreamAgSense::khash(SML_VI_IRC_IDX); // "irc" // IR Cut (boolean)


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
    numRecords = 0;
    cmp_init(&cmp, (void*) _stream, SenMLStream::stream_reader, SenMLStream::stream_writer);
}

void SenMLStream::begin(int baud){
      _sending = false;
      _stream->begin(baud);
}

bool SenMLStream::parseField(string key,int r) {
    string sval;
    float fval;

    if ( IS_KEY(key,SML_NAME) ||
         IS_KEY(key,SML_UNIT) ||
         IS_KEY(key,SML_LINK) ||
         IS_KEY(key,SML_STR_VALUE) ) {
        printf("Reading Key [%s]",key.c_str());
        if (!readString(sval,SML_VAL_SIZE))
            return false;
        put(key,sval,r);
    }

    if ( IS_KEY(key,SML_VALUE) ||
         IS_KEY(key,SML_SUM_VALUE) ||
         IS_KEY(key,SML_UPDATE_TIME) ||
         IS_KEY(key,SML_BOOL_VALUE) ||
         IS_KEY(key,SML_TIME) ) {
        if (!readNumber(fval))
            return false;
        printf("Adding Number  [%f]\n",fval);
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


bool SenMLStream::parseRecord(int r) {
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
            if (!parseHeader())
                return false;
            #ifdef SMLDEBUG
            printf("parsed header\n");
            #endif
        } else {      
            if (!parseRecord(i))
                return false;
            #ifdef SMLDEBUG
            printf("parsed record %d\n",i);
            #endif
            numRecords++;
        }
        fflush(stdout); 
    }
    return available();
}

