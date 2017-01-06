#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "../senmlstream.h"

BufferedEscapedLinuxSerialWrapper b;


uint8_t buff[32];
    
int main(int argc, char **argv) {
    SenMLStreamAgSense s(&b);
    s.begin(9600);
    string key,val;
    float fval =0;
    bool bval = false;
    
    while(1) {
        if (s.loop()){ // Must have record
            s.print();
            printf("done print\n");
            s.writeSenML(2); //  header + 1 extra record
            
            s.get(s.SML_BASENAME,val);
            s.appendRecord(1); // 1 maps
            s.appendMap(s.SML_BASENAME,val);

            s.appendRecord(3); // 3 maps

            s.get(s.SML_NAME,val,1);
            s.appendMap(s.SML_NAME,val);
            
            s.get(s.SML_VALUE,fval,1);
            s.appendMap(s.SML_VALUE,fval);

            s.get(s.SML_BOOL_VALUE,bval,1);
            s.appendMap(s.SML_BOOL_VALUE,bval);

            s.flush();

            s.reset();
            printf("done reset\n\n\n\n");
        }
    }
    return 0;
}
