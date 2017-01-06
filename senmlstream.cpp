#include "senmlstream.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


const string SenMLStream::keymaps[]  = {SML_BASENAME,
SML_BASETIME,
SML_BASEUNIT,
SML_BASEVALUE,
SML_BASESUM  ,
SML_VERSION  ,
SML_NAME,
SML_UNIT,
SML_VALUE,
SML_STR_VALUE,
SML_BOOL_VALUE,
SML_DATA_VALUE,
SML_POSITION,
SML_UPD_TIME,
SML_TIME,
SML_LINK,
SML_SUM_VALUE,
"_"};

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
         IS_KEY(key,SML_UPD_TIME) ||
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

