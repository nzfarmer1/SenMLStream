# SenMLStream

Work in progress

SenMLParser being developed for Arduino. Currently working with the Linux Serial emulator:

https://github.com/nzfarmer1/arduino-serial

Requires:

https://github.com/nzfarmer1/HashMap

(assumes these are located at same level in folder heirarchy)

Uses msgpack encoding rather than JSON, and accepts SenML requests in SenML over serial rather than CoAP.

Running tests

- make test
- Run arduino-serial/socat.sh *
- cd  test && npm install .
- Run test/test  *
- Run nodejs/client.js -s &lt;fullpath to socket&gt; senml.js *

* in seperate terminal session or disown

Building:

Add -D_SIMULATOR to run in Linux/OSX mode.  For Arduino mode, it requires the xarq libraries that are currently private.  Please get in touch with the author if you wish to license them for use.

Andrew McClure (AgSense Ltd.) http://facebook.com/agsensenz





  


