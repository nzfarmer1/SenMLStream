#! /usr/bin/env node

    const net = require("net");
    const mp = require("msgpack5")();
    const s = new require('stream').PassThrough({objectMode:true});
    const bl = new require('bl');
    const fs = require("fs");
    const stdio = require("stdio");
    const stream = require("stream");
    const encode = require("simplepacket").encode();
    const decode = require("simplepacket").decode();

	var opts = stdio.getopt({
    	'socket': {key: 's', args: 1,mandatory:true, description: 'Socket File'},
    	'noexit': {key: 'f', args: 0,mandatory:false, description: 'No exit after first response'},
	});


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
        console.log(opts);
        var e = eval(contents.toString());
        console.log(e);
        msg = mp.encode(e);
    
        var obuffer = new stream.PassThrough();
        var ibuffer = encode;
   
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
           return;
        });

        var buffer = Buffer(0);
        var stdout = process.stdout;
        obuffer.pipe(decode,{end: false});
        
        decode.on('data',(data)=>{
           buffer =Buffer.concat([buffer,data]);
        });
                
        decode.on("flush",()=>{
            console.log("Flush");
            console.log(buffer);
            console.log(mp.decode(buffer));
            process.exit(0);
//            buffer = Buffer(0);
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

        c.setTimeout(200);

        c.on('timeout',()=>{
            ;//console.log("timeout");
        });	

        c.on('data',(data)=>{
              obuffer.push(data);
        }); 	
 
       //var sm =  mp.decode(data);
 
