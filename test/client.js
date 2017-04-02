#! /usr/bin/env node

    const net = require("net");
    const mp = require("msgpack5")();
    const s = new require('stream').PassThrough({objectMode:true});
    const bl = new require('bl');
    const fs = require("fs");
    const stdio = require("stdio");
    const stream = require("stream");
    const encode = require("simplepacket").encode;
    const decode = require("simplepacket").decode;

	var opts = stdio.getopt({
    	'socket': {key: 's', args: 1,mandatory:true, description: 'Socket File'},
    	'noexit': {key: 'f', args: 0,mandatory:false, description: 'No exit after first response'},
	});

    console.log(opts);

    var obuffer = new stream.PassThrough();
    var ibuffer = encode();


// Initiate the source

      const error = function(msg){
	var _msg = (msg) ? msg : "Usage: " + __filename + " <use case.js>";
	console.error(_msg);
    	process.exit(-1);
      };

     if (process.argv.length <= 2) {
	error();
     } 
     
      var msg; 
      var contents = fs.readFileSync(opts.args[0]);
	  var e = eval(contents.toString());
      console.log(e);
	    //console.log(mp.decode(mp.encode(e)));
      msg = mp.encode(e);
	
      const socketPath = opts.socket;
      const c = net.createConnection(socketPath);
      var conn;
      c.setTimeout(100);
      c.on('close', (e) => {
			process.exit(0);
        conn = null;
        return;
      });
      c.on('error', (e) => {
		    console.log("error " + e.toString());
			process.exit(0);
        conn = null;
        return;
      });
      ibuffer.on('data',(d) => {console.log('x',d); c.write(d); });
      c.on('connect', () => {
        conn = c;
        console.log("connected");
	console.log("Sending ...");
	//console.log(msg);
	ibuffer.write(msg);
	ibuffer.flush();
	setInterval(()=>{
		//process.exit(0);
	},1000);
      });
      c.on('data',(data)=>{
	var sm =  mp.decode(data);
	    console.log(sm);
        if (!opts.noexit)
	        process.exit(0);
      }); 	
