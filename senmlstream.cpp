#include "senmlstream.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


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

void SenMLStream::flush(){
    if (_sending)
        _stream->endPacket();
    _sending = false;
};

void SenMLStream::setMaxBufAllowed(unsigned int max) {
    //max_buf_allowed = max;
}


bool SenMLStream::parseHeader() {
    uint32_t string_size,msize;
    char key[SML_KEY_SIZE],val[SML_VAL_SIZE];

    sH.reset();
    
    if (cmp_read_map(&cmp,&msize)){
        for(uint32_t j=0; j<msize;j++){
            #ifdef SMLDEBUG
            printf("msize =>%d\n",msize);
            #endif
            if (msize >2)
                continue; //wtf?
            if (!cmp_read_str_size(&cmp, &string_size))
                return false;
            if (string_size > SML_KEY_SIZE)
                return false;
            if (!_stream->read((uint8_t*)key, string_size))
                return false;
            key[string_size] = '\0';
            string k(key);

            #ifdef SMLDEBUG
            printf("Got key %s\n",key);
            #endif
            
            if (k == SML_BASENAME){
                if (!cmp_read_str_size(&cmp, &string_size))
                    return false;
                if (string_size > SML_VAL_SIZE)
                    return false;
                if (!_stream->read((uint8_t*)val, string_size))
                    return false;
            #ifdef SMLDEBUG
                printf("Setting BN %s\n",val);
            #endif
                val[string_size] = '\0';
                sH.setBN(string(val));
            }
        }
    }

    return msize >0;
}


bool SenMLStream::parseVI(SenMLValueInfo &vi,uint8_t msize) {
    uint32_t string_size;
    cmp_object_t obj;
    char key[SML_KEY_SIZE];
    printf("Parse VI\n");
        
    for(uint32_t j=0; j<msize;j++){
        if (!cmp_read_str_size(&cmp, &string_size))
            return false;
        if (string_size > SML_KEY_SIZE)
            return false;
        if (!_stream->read((uint8_t*)key, string_size))
            return false;
        key[string_size] = '\0';

        if (!cmp_read_object(&cmp, &obj))
            return false;

        printf("VI:Type => %x Key %s\n" ,obj.type,key);
        switch(obj.type) {
            case CMP_TYPE_POSITIVE_FIXNUM:
            case CMP_TYPE_UINT8:
                if IF_KEY(key,SML_VI_EXP)
                    vi.exp = (uint16_t)obj.as.u8;
                if IF_KEY(key,SML_VI_RES)
                    vi.res = (uint8_t)obj.as.u8;
                break;
            case CMP_TYPE_UINT16:
                if IF_KEY(key,"exp")
                    vi.exp = (uint16_t)obj.as.u16;
                if IF_KEY(key,"res")
                    vi.res = (uint8_t)obj.as.u16;
                break;
            case CMP_TYPE_BOOLEAN:
                if IF_KEY(key,"irc")
                    vi.irc = (bool)obj.as.boolean;
                break;
            default:
                printf("Unknown type while processing VI %x\n",obj.type);
        }
    }
    
    return true;    
}


bool SenMLStream::parseRecord() {
    uint32_t msize,string_size;
    cmp_object_t obj;
    char key[SML_KEY_SIZE],val[SML_VAL_SIZE];
    
    SenMLRecord * sR = createRecord();

    if (!sR)
        return false;

    if (cmp_read_map(&cmp,&msize)){
        for(uint32_t j=0; j<msize;j++){
            if (!cmp_read_str_size(&cmp, &string_size))
                return false;
            if (string_size > SML_KEY_SIZE)
                return false;
            if (!_stream->read((uint8_t*)key, string_size))
                return false;
            key[string_size] = '\0';
            string k(key);
            #ifdef SMLDEBUG
            printf("parse record - got key %s\n",key);
            #endif

            if (cmp_read_object(&cmp, &obj)) {
               #ifdef SMLDEBUG
                printf("Object found %x %d\n",obj.type,obj.as.str_size);
                #endif
                switch(obj.type) {
                    case CMP_TYPE_POSITIVE_FIXNUM:
                        #ifdef SMLDEBUG
                        printf("Got FIXNUM: %d\n",obj.as.u8);
                        #endif 
                        break;
                    case CMP_TYPE_FIXMAP:
                            if ( k == SML_INFO) {
                                printf("Parse VI? %s\n",string(key).c_str());
                                parseVI(sR->getValueInfo(),obj.as.map_size);
                            }
                        break;
                    case CMP_TYPE_FIXSTR:
                        if (_stream->read((uint8_t*)val, obj.as.str_size)) {
                            val[obj.as.str_size] = '\0';
                            #ifdef SMLDEBUG
                            printf("Got FIXSTR %s\n",val);
                            #endif
        
                            if (k == SML_NAME ){
                                sR->setName(string(val));
                            } else
                            if (k == SML_UNIT ){
                                sR->setUnit(string(val));
                            }
                        }
                        break;
                    case CMP_TYPE_FLOAT:
                        #ifdef SMLDEBUG
                        printf("%f\n",obj.as.flt);
                        #endif
                        if (k == SML_VALUE ) {
                            sR->setValue(obj.as.flt);
                        } else
                        if (k == SML_MIN ) {
                            sR->setMin(obj.as.flt);
                        } else
                        if (k == SML_MAX ) {
                            sR->setMin(obj.as.flt);
                        }
                       break;
                    default:
                      //  printf("Unsupported %x\n",obj.type);
                        return false;
                        break;
                }
            } else {
                ;//printf("CMP Error: %d\n",cmp.error);
            }
        }
    }
    return true;    
}


bool SenMLStream::loop() {
    uint32_t asize;
    
    if (!cmp_read_array(&cmp, &asize)){
        return false;
    }

    // parse record
    this->reset();
    for(uint32_t i=0;i<asize;i++){
            if (!i){
                if (!parseHeader())
                    return false;
                #ifdef SMLDEBUG
                printf("parsed header\n");
                #endif
            } 
            if (i > 0) {      
                if (!parseRecord())
                    return false;
                #ifdef SMLDEBUG
                printf("parsed record %d\n",i);
                #endif
            }
            fflush(stdout); 
    }
    return available();
}

