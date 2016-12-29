#include "senmlstream.h"
#include <stdlib.h>
#include <stdio.h>

#include "cmp.h"


bool SenMLStream::stream_reader(cmp_ctx_t * ctx, void * data, size_t limit){

    if (((BufferedEscapedLinuxSerialWrapper*) ctx->buf)->available(limit)) {
        return ((BufferedEscapedLinuxSerialWrapper*) ctx->buf)->read((uint8_t*)data,limit);
    }
    return false;
}

size_t SenMLStream::stream_writer(cmp_ctx_t * ctx, const void * data, size_t count){
        return ((BufferedEscapedLinuxSerialWrapper*) ctx->buf)->write((uint8_t*)data,count);
}

 
SenMLStream::SenMLStream(BufferedEscapedLinuxSerialWrapper * stream) : _stream(stream)  {
    numRecords = 0;
    cmp_init(&cmp, (void*) _stream, SenMLStream::stream_reader, SenMLStream::stream_writer);
}

bool SenMLStream::begin(int baud){
      _sending = false;
      return  _stream->begin(baud);
}

void SenMLStream::flush(){
    if (_sending)
        _stream->endPacket();
    _sending = false;
};

void SenMLStream::setMaxBufAllowed(unsigned int max) {
    //max_buf_allowed = max;
}


boolean SenMLStream::parseHeader() {
    uint32_t string_size,msize;
    char key[3],val[256];

    sH.reset();
    
    if (cmp_read_map(&cmp,&msize)){
        for(uint32_t j=0; j<msize;j++){
            printf("msize =>%d\n",msize);
            if (msize >2)
                continue; //wtf?
            if (!cmp_read_str_size(&cmp, &string_size))
                return false;
            if (!_stream->read((uint8_t*)key, string_size))
                return false;
            key[string_size] = '\0';
            string k(key);
            printf("Got key %s\n",key);
            
            if (k == "bn"){
                if (!cmp_read_str_size(&cmp, &string_size))
                    return false;
                if (!_stream->read((uint8_t*)val, string_size))
                    return false;
                printf("Setting BN %s\n",val);
                val[string_size] = '\0';
                sH.setBN(string(val));
            }
        }
    }

    return msize >0;
}

boolean SenMLStream::parseRecord() {
    uint32_t msize,string_size;
    cmp_object_t obj;
    char key[3],val[256];
    
    SenMLRecord * sR = createRecord();

    if (!sR)
        return false;
    printf("Parse record\n");
    //Read Name

        if (cmp_read_map(&cmp,&msize)){
            for(uint32_t j=0; j<msize;j++){
                if (!cmp_read_str_size(&cmp, &string_size))
                    return false;
                if (!_stream->read((uint8_t*)key, string_size))
                    return false;
                key[string_size] = '\0';
                string k(key);
                printf("parse record - got key %s\n",key);
    
                if (cmp_read_object(&cmp, &obj)) {
                    printf("Object found %x %d\n",obj.type,obj.as.str_size);
                    switch(obj.type) {
                        case CMP_TYPE_POSITIVE_FIXNUM:
                            printf("%d\n",obj.as.u8);
                            break;
                        case CMP_TYPE_FIXSTR:
                            if (_stream->read((uint8_t*)val, obj.as.str_size)) {
                                val[obj.as.str_size] = '\0';
            
                                printf("%s\n",val);
            
                                if (k == "n" ){
                                    sR->setName(string(val));
                                } else
                                if (k == "u" ){
                                    sR->setUnit(string(val));
                                }
                            }
                            break;
                        case CMP_TYPE_FLOAT:
                            printf("%f\n",obj.as.flt);
                            if (k == "v" ) {
                                sR->setValue(obj.as.flt);
                            } else
                            if (k == "min" ) {
                                sR->setMin(obj.as.flt);
                            } else
                            if (k == "max" ) {
                                sR->setMin(obj.as.flt);
                            }
                           break;
                        default:
                            printf("Unsupported %x\n",obj.type);
                            return false;
                            break;
                    }
                } else {
                    printf("CMP Error: %d\n",cmp.error);
                }
            }
        }
    return true;    
}


boolean SenMLStream::loop() {
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
                printf("parsed header\n");
            } 
            if (i > 0) {      
                if (!parseRecord())
                    return false;
                printf("parsed record %d\n",i);
            }
                fflush(stdout); 
    }
    return available();
}

