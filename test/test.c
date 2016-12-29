#include <stdlib.h>
#include <stdio.h>
#include "../senmlstream.h"

BufferedEscapedLinuxSerialWrapper b;


int main(int argc, char **argv){
    SenMLHeader sH;
    SenMLRecord r(&sH);
    SenMLStream s(&b);
    s.begin(9600);
    while(1)
        if (s.loop()){ // Must have record
            s.print();
            r.clone(s.record(0));
            s.reset();
            s.addRecord(&r);
	    s.print();	
            s.writeSenML(&r);
            s.appendMaps(3);
            s.appendMap(SML_NAME,r.getName());
            s.appendMap(SML_VALUE,r.getValue());
            s.appendMap(SML_UNIT,r.getUnit());
            s.flush();
        }

    return 0;
}
