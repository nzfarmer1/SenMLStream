# SenMLStream

Work in progress

SenMLParser being developed for Arduino. Currently working with the Linux Serial emulator:

https://github.com/nzfarmer1/arduino-serial

Further reading on our implementation of SenML here:

https://www.linkedin.com/pulse/developing-protocol-our-900mhz-farm-telemetry-solution-andrew-mcclure

For details on SenML see:

https://datatracker.ietf.org/doc/draft-ietf-core-senml/

Requires:

https://github.com/nzfarmer1/HashMap

cmp message pack parser bundled but original is here:
(assumes these are located at same level in folder heirarchy)
../HashMap
../arduino-serial
../SenMLStream

https://github.com/camgunz/cmp

Uses msgpack encoding rather than JSON, and accepts SenML requests in SenML over serial rather than CoAP.

Running tests

- cd  test && npm install .
- Run arduino-serial/socat.sh
- make test
- Run test/test  // uses /tmp/ttysocat0 by default See ../../arduino-serial/linux-serial.h 
- Run test/client.js -s /tmp/test.sock  senml.js   // ( see socat.sh  - need to wait before sending)

Building:

Add -D_SIMULATOR to run in Linux/OSX mode.  For Arduino mode, it requires the xarq libraries that are currently private.  Please get in touch with the author if you wish to license them for use.

Compile linux-serial with -DSERIALPORTDEBUG to monitor tty serial
Comiple with -D__SMLDEBUG_LINUX to monitor SmlStream debugging on Linux

Andrew McClure (AgSense Ltd.) http://facebook.com/agsensenz





  


