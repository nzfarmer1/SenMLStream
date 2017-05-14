#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "../senmlstream.h"

BufferedEscapedLinuxSerialWrapper b;

uint8_t buff[32];
int main(int argc, char **argv) {
    SenMLStreamAgSense s(&b);
    s.begin(9600);
    printf("Startup\n");
    s.setBN(string("urn:dev:xbee:0xFF"));
    string key,val;
    float fval =0;
    bool bval = false;
    string bn,ibn;
    int res;
    while(1) {
        if (s.loop()){ // Must have record
            printf("Got message");
            s.print();
            printf("done print\n");
            res = s.get(s.SML_BASENAME,ibn));
            s.writeSenML(1,ibn); //  header + 1 extra record. if ibn == "" default bn used
      
            s.appendRecord(3); // 3 maps

            s.get(s.SML_NAME,val,1);
            s.appendMap(s.SML_NAME,val);
            
            s.get(s.SML_VALUE,fval,1);
            s.appendMap(s.SML_VALUE,fval);

            s.get(s.SML_BOOL_VALUE,bval,1);
            s.appendMap(s.SML_BOOL_VALUE,bval);
            
            s.flush();

            s.reset();
        }
    }
    return 0;
}
