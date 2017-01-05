#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "../senmlstream.h"

BufferedEscapedLinuxSerialWrapper b;

const char * filename = "/Users/andrew/Pictures/family.jpg";
char *st = "foobar test hello world 5678901\0";

uint8_t buff[32];
    
int main(int argc, char **argv) {
    SenMLHeader sH;
    SenMLRecord r(&sH);
    SenMLStream s(&b);
    s.begin(9600);
    int fp = open(filename,O_RDONLY);
    if (fp <=0)
        exit(0);
    uint32_t fs =0,fc;
    while(true){
        fc = read(fp,buff,32);
        fs+=fc;
        if (fc < 32)
            break;
    }
//    while((fs += read(fp,buff,32)) >0){;}
    printf("len => %d\n",fs);
    while(1) {
        if (s.loop()){ // Must have record
            lseek(fp,0,SEEK_SET);
            s.print();
            r.clone(s.record(0));
            s.reset();
            s.addRecord(&r);
            s.print();	
            s.writeSenML(&r,1); // default 1 record
            s.appendRecord(2); // 3 maps
            s.appendMap(SML_NAME,"cam");
            s.appendMap(SML_DATA_VALUE,(uint8_t*)buff,fs);
            while(true){
                fc = read(fp,buff,32);
                if (fc>0)
                    s.appendBinary(buff,fc);
                if (fc < 32)
                    break;
            }
 
            s.flush();
            s.print();
            exit(0);
        }
    }

    return 0;
}
